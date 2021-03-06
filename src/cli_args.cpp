#include "cli_args.hpp"
#include "cxxopts.hpp"
#include <fmt/core.h>

static constexpr const char *name = "cw39";
static constexpr const char *vers = "v2.2-dev";
static constexpr const char *desc =
        "An optimizing compiler for the subset of the C language.\n"
        "Created by Mikhail Sokolovskiy in 2022.\n"
        "More info: https://github.com/Sokolmish/diploma\n";

static constexpr const char *argPosFilename = "inputFile";

static constexpr const char *argPreproc = "pproc";
static constexpr const char *argAst = "ast";
static constexpr const char *argIr = "ir";
static constexpr const char *argCfg = "cfg";
static constexpr const char *argLLVM = "llvm";
static constexpr const char *argBC = "bc";
static constexpr const char *argASM = "asm";

static constexpr const char *argOpt = "O";
static constexpr const char *argAddDefine = "D";
static constexpr const char *argLLCArgs = "llc-args";
static constexpr const char *argNoS1 = "no-s1";
static constexpr const char *argNoS2 = "no-s2";

static constexpr const char *argTraceScanner = "tr-scanner";
static constexpr const char *argTraceParser = "tr-parser";
static constexpr const char *argTimes = "times";

static constexpr const char *envLLC = "CW39_LLC";
static constexpr const char *defaultLLC = "llc";

class CLIArgs::CLIArgs_Impl {
public:
    cxxopts::Options options;
    cxxopts::ParseResult res;

    CLIArgs_Impl(int argc, char **argv);
};

CLIArgs::CLIArgs_Impl::CLIArgs_Impl(int argc, char **argv) : options(name, desc) {
    using namespace cxxopts;

    options.custom_help("[options]");
    options.positional_help("<filename>");

    options.add_options("")(argPosFilename, "Input file", value<std::string>());
    options.parse_positional(argPosFilename);

    std::vector<std::string> helpGroups;

    auto opt1 = options.add_options("Output");
    helpGroups.push_back("Output");

    opt1(argPreproc, "Write preprocessor output", value<std::string>()->implicit_value(""), "path");
    opt1(argAst, "Write AST", value<std::string>()->implicit_value(""), "path");
    opt1(argIr, "Write IR after optimizations", value<std::string>()->implicit_value(""), "path");
    opt1(argCfg, "Write CFG after optimizations", value<std::string>()->implicit_value(""), "path");
    opt1(argLLVM, "Write LLVM assembly", value<std::string>()->implicit_value(""), "path");
    opt1(argBC, "Write LLVM bitcode", value<std::string>()->implicit_value(""), "path");
    opt1(argASM, "Write assembly code generated by llc", value<std::string>()->implicit_value(""), "path");

    auto opt2 = options.add_options("Compilation");
    helpGroups.push_back("Compilation");

    opt2(argAddDefine, "Add preprocessor macro", value<std::vector<std::string>>(), "<macro>");
    opt2(argOpt, "Set optimization level from 0 to 2", value<uint>()->default_value("2"), "<lvl>");
    opt2(argNoS1, "Disable S1 optimization");
    opt2(argNoS2, "Disable S2 optimization");
    opt2(argLLCArgs, "Specify llc args", value<std::string>(), "<args>");

    auto opt3 = options.add_options("Debug");
    helpGroups.push_back("Debug");

    opt3(argTimes, "Print elapsed time of each step into stderr");
    opt3(argTraceScanner, "Enable scanner tracing");
    opt3(argTraceParser, "Enable parser tracing");

    auto optI = options.add_options("Info");
    helpGroups.push_back("Info");

    optI("version", "Print version");
    optI("h,help", "Print usage");

    try {
        res = options.parse(argc, argv);
        if (!res.unmatched().empty()) {
            fmt::print("Options error: Unexpected argument: {}\n", res.unmatched()[0]);
            exit(EXIT_FAILURE);
        }
    }
    catch (cxxopts::OptionException const &exc) {
        fmt::print(stderr, "Options error: {}\n", exc.what());
        exit(EXIT_FAILURE);
    }

    if (res.count("help")) {
        fmt::print("{}\n", options.help(helpGroups));
        exit(EXIT_SUCCESS);
    }
    else if (res.count("version")) {
        fmt::print("{} version: {}\n", name, vers);
        exit(EXIT_SUCCESS);
    }
}

CLIArgs::CLIArgs(int argc, char **argv)
        : impl(std::make_unique<CLIArgs_Impl>(argc, argv)) {}

CLIArgs::~CLIArgs() = default;

// Optput options getters

std::optional<std::string> CLIArgs::getOutParam(const char *pname) const {
    if (impl->res.count(pname))
        return impl->res[pname].as<std::string>();
    return std::nullopt;
}

std::optional<std::string> CLIArgs::outPreproc() const {
    return getOutParam(argPreproc);
}

std::optional<std::string> CLIArgs::outAST() const {
    return getOutParam(argAst);
}

std::optional<std::string> CLIArgs::outIR() const {
    return getOutParam(argIr);
}

std::optional<std::string> CLIArgs::outCFG() const {
    return getOutParam(argCfg);
}

std::optional<std::string> CLIArgs::outLLVM() const {
    return getOutParam(argLLVM);
}

std::optional<std::string> CLIArgs::outBC() const {
    return getOutParam(argBC);
}

std::optional<std::string> CLIArgs::outASM() const {
    return getOutParam(argASM);
}

// Compilation options getters

uint32_t CLIArgs::getOptLevel() const {
    return impl->res[argOpt].as<uint>();
}

std::vector<std::string> CLIArgs::getDefines() const {
    if (!impl->res.count(argAddDefine))
        return {};
    return impl->res[argAddDefine].as<std::vector<std::string>>();
}

bool CLIArgs::isS1_Enabled() const {
    return !impl->res.count(argNoS1);
}

bool CLIArgs::isS2_Enabled() const {
    return !impl->res.count(argNoS2);
}

std::optional<std::string> CLIArgs::getLLCArgs() const {
    if (impl->res.count(argLLCArgs))
        return impl->res[argLLCArgs].as<std::string>();
    return std::nullopt;
}

// Debug options getters

bool CLIArgs::isScannerTracing() const {
    return impl->res.count(argTraceScanner);
}

bool CLIArgs::isParserTracing() const {
    return impl->res.count(argTraceParser);
}

bool CLIArgs::isShowTimes() const {
    return impl->res.count(argTimes);
}

// Other getters + env

std::string CLIArgs::inputFile() const {
    if (!impl->res.count(argPosFilename))
        return "";
    return impl->res[argPosFilename].as<std::string>();
}


std::string CLIArgs::get_llc_name() const {
    const char *envVal = getenv(envLLC);
    if (envVal == nullptr || *envVal == '\0')
        return defaultLLC;
    return envVal;
}

std::string CLIArgs::getSelfVersion() {
    return vers;
}
