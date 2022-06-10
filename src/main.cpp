#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <fmt/core.h>
#include <fmt/chrono.h>

#include "cli_args.hpp"

#include "parser/preprocessor.hpp"
#include "parser/core_driver.hpp"
#include "ir/generator.hpp"

#include "transformations/vars_virtualizer.hpp"
#include "transformations/ssa_generator.hpp"
#include "transformations/computing_transformers.hpp"
#include "transformations/global_transformers.hpp"
#include "transformations/loop_inv_mover.hpp"
#include "transformations/intrinsics_detector.hpp"

#include "ir_2_llvm.hpp"

#include "subprocess.hpp"

/** If path is not empty then write to it, else write to stdout */
static void writeOut(std::string const &path, std::string const &data) {
    if (path.empty() || path == "-") {
        fmt::print("{}\n", data);
    }
    else {
        std::ofstream file(path);
        if (!file.is_open())
            throw cw39_error(fmt::format("Cannot open file '{}'", path));
        file << data << std::endl;
    }
}


static void writeOutBinary(std::string const &path, char const *data, size_t size) {
    if (path.empty() || path == "-") {
#ifdef __unix__
        if (path.empty() && isatty(fileno(stdout))) {
            throw cw39_error("Bitcode is not supposed to be written into terminal.\n"
                             "Use `--bc=-` to force this (can break something).");
        }
        FILE *out = fdopen(dup(fileno(stdout)), "wb");
        fwrite(data, 1, size, out); // TODO: missed check
        fclose(out);
#else
        throw cw39_error("Binary output into stdout is not available for current platform");
#endif
    }
    else {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
            throw cw39_error(fmt::format("Cannot open file '{}'", path));
        file.write(data, size);
    }
}

enum class CompilationLevel {
    PREPROCESS,     // files -> text
    PARSE,          // text -> ast
    GENERATE,       // ast -> ir (+optimization)
    MATERIALIZE,    // ir -> llvm
    COMPILE,        // llvm -> asm
};

static CompilationLevel getCompilationLvl(CLIArgs const &args) {
    if (args.outASM())
        return CompilationLevel::COMPILE;
    if (args.outLLVM() || args.outBC())
        return CompilationLevel::MATERIALIZE;
    else if (args.outIR() || args.outCFG())
        return CompilationLevel::GENERATE;
    else if (args.outAST())
        return CompilationLevel::PARSE;
    else if (args.outPreproc())
        return CompilationLevel::PREPROCESS;
    else
        return CompilationLevel::MATERIALIZE; // Default level
}

static void optimizeFunction(IntermediateUnit::Function &func, CLIArgs const &args) {
    if (args.getOptLevel() == 0)
        return;
    CFGraph cfg = std::move(func.cfg);
    cfg = VarsVirtualizer(std::move(cfg)).moveCfg();
    cfg = SSA_Generator(std::move(cfg)).moveCfg();
    cfg = AlgebraicTransformer(std::move(cfg)).moveCfg();
    cfg = CommonSubexprElim(std::move(cfg)).moveCfg();
    cfg = CopyPropagator(std::move(cfg)).moveCfg();
    cfg = TailrecEliminator(std::move(cfg), func.getId()).moveCfg();

    if (args.getOptLevel() >= 2) {
        cfg = FunctionsInliner(std::move(cfg)).moveCfg();
        cfg = LoopInvMover(std::move(cfg)).moveCfg();
        if (args.isS1_Enabled()) {
            cfg = IntrinsicsDetector(std::move(cfg)).moveCfg();
            cfg = CommonSubexprElim(std::move(cfg)).moveCfg(); // TODO: temporary
        }
    }

    func.cfg = std::move(cfg);
}

// I didn't even try to write these long chrono types
template <typename T>
static void printElapsedTime(std::string const &name, T const &time) {
    namespace chrono = std::chrono;
    auto ms = chrono::duration_cast<chrono::milliseconds>(time);
    auto us = chrono::duration_cast<chrono::microseconds>(time);
    fmt::print(stderr, "{:13s}: {:5} ({})\n", name, ms, us);
}

static void process(CLIArgs  &args) {
    using steady_clock = std::chrono::steady_clock;
    steady_clock::time_point startTime, stopTime;

    auto compilationLvl = getCompilationLvl(args);

    if (args.inputFile().empty())
        throw cw39_error("No input file");

    startTime = steady_clock::now();
    Preprocessor preproc(args.inputFile(), args.getDefines());
    stopTime = steady_clock::now();

    std::string text = preproc.getText();
    auto ctx = preproc.getContext();

    if (args.outPreproc())
        writeOut(*args.outPreproc(), text);
    if (args.isShowTimes())
        printElapsedTime("Preprocessor", stopTime - startTime);

    if (compilationLvl <= CompilationLevel::PREPROCESS)
        return;

    int parserDebugFlags = 0;
    if (args.isScannerTracing())
        parserDebugFlags |= CoreDriver::TRACE_SCANNER;
    if (args.isParserTracing())
        parserDebugFlags |= CoreDriver::TRACE_PARSER;
    std::string textCopy = text;

    startTime = steady_clock::now();
    CoreDriver parser(*ctx, std::move(textCopy), parserDebugFlags);
    stopTime = steady_clock::now();

    std::shared_ptr<AbstractSyntaxTree> ast = parser.getAST();
    if (args.outAST())
        writeOut(*args.outAST(), ast->top->getTreeNode(*ctx)->printHor());
    if (args.isShowTimes())
        printElapsedTime("Parser", stopTime - startTime);

    if (compilationLvl <= CompilationLevel::PARSE)
        return;

    startTime = steady_clock::now();
    IR_Generator gen(*ast, *ctx);
    stopTime = steady_clock::now();

    if (args.isShowTimes())
        printElapsedTime("Generator", stopTime - startTime);

    auto rawUnit = gen.getIR();
    IntermediateUnit optUnit = *rawUnit;

    startTime = steady_clock::now();
    for (auto &[fId, func] : optUnit.getFuncsMut()) {
        optimizeFunction(func, args);
    }
    stopTime = steady_clock::now();

    if (args.outIR())
        writeOut(*args.outIR(), optUnit.printIR());
    if (args.outCFG())
        writeOut(*args.outCFG(), optUnit.drawCFG());
    if (args.isShowTimes())
        printElapsedTime("Optimizator", stopTime - startTime);

    if (compilationLvl <= CompilationLevel::GENERATE)
        return;

    startTime = steady_clock::now();
    IR2LLVM materializer(optUnit);
    stopTime = steady_clock::now();

    if (args.outLLVM())
        writeOut(*args.outLLVM(), materializer.getLLVM_IR());
    if (args.outBC()) {
        auto const &data = materializer.getLLVM_BC();
        writeOutBinary(*args.outBC(), data.data(), data.size());
    }
    if (args.isShowTimes())
        printElapsedTime("Materializer", stopTime - startTime);

    if (compilationLvl <= CompilationLevel::MATERIALIZE)
        return;

    if (args.outASM()) {
        using namespace subprocess;

        std::string llc_args = args.get_llc_name();
        if (args.getLLCArgs()) {
            llc_args += " " + *args.getLLCArgs(); // TODO: doesn't handle quoted args
        }
        else { // Default LLC arguments
            llc_args += " -O0 -mcpu=native";
        }

        auto p = Popen(llc_args, input(PIPE), output(PIPE)); // TODO: handle exceptions here
        auto &llvmIr = materializer.getLLVM_IR(); // Can send bitcode
        auto res = p.communicate(llvmIr.c_str(), llvmIr.size());
        auto asmStr(res.first.buf.data());

        writeOut(*args.outASM(), asmStr);
    }

    if (compilationLvl <= CompilationLevel::COMPILE)
        return;
}

int main(int argc, char **argv) {
    CLIArgs args(argc, argv);
    try {
        process(args);
    }
    catch (cw39_exception &exc) {
        std::cerr << exc.prettyWhat();
        return EXIT_FAILURE;
    }
    catch (std::exception &exc) {
        std::cerr << "Something went wrong:\n\t" << exc.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
