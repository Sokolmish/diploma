# Diploma

This is an optimizing compiler for the subset of the C99 language producing LLVM assembler.
It is my bachelor's diploma work.

There are some examples of compilable code in the [test directory](/tests).
All tests can be checked via [run_tests](/tests/run_tests.sh) script (one should set the
path to the compiler executable via `CC_tst` variable inside this script).

Following optimizations are implemented in this compiler:

- Memmory to registers conversion, SSA-form creating
- Tail recursion elimination
- Dead code elimination
- Copy propagation and constants folding
- Algebraic simplifications and common subexpressions elimination
- Loop invariant code motion
- Intrisics introduction (special optimization, *CTZ* only)

## Requirements

- **CMake**
- **gcc-10** and above
- **LLVM-13** and above (because early versions cannot be compiled with C++20)
- **[fmtlib](https://fmt.dev/latest/index.html)** (tested with versions 6 and 8)

Following packages are used for code generation.
They are optional because all code, that should be generated by them, are already included in this repository.

- [**Flex**](https://github.com/westes/flex) (v2.6.4) and [**Bison**](https://www.gnu.org/software/bison) (v3.8.1)
- [**Gperf**](https://www.gnu.org/software/gperf)

## Building

To build this compiler one should execute following commands, which are common for most CMake projects.
Executable binary (`cw39`) will be placed into the `build` directory.

```sh
mkdir build
cd build
cmake ..
make
```

By default, all code generators are enabled.
But if you don't have such generators installed, the project can be built using already generated source files.
To do this, you should call CMake with some of following options:

- `-DCW39_NO_BISON=TRUE` to disable flex and bison
- `-DCW39_NO_GPERF=TRUE` to disable gperf

For example, cmake command can be executed as shown below.

```
cmake -DCW39_NO_BISON=TRUE -DCW39_NO_GPERF=TRUE ..
```

Also, if you have troubles with CMake and generators communication,
you can generate source codes manually using this [Makefile](/src/parser/Makefile).

### Building using Docker

Docker container can be built using the [`setup.sh`](/setup.sh) script. 
After the first usage, this command should be executed if [Dockerfile](/Dockerfile) has been changed.

Project can be built using the [`build.sh`](/build.sh) script.
This script will store the final binary and building files in the `docker_build` directory created by previous script.

Both scripts don't expect arguments and should be executed from project root (where Dockerfile is located).
Optionally, one can specify number of threads used by make in the build script (default is 3).

## Usage

```
cw39 [options] <input_file>
```

### Output options

| Option    | Description                                                     |
|-----------|-----------------------------------------------------------------|
| `--pproc` | Preprocessor output                                             |
| `--ast`   | Abstract syntax tree with pseudo graphics                       |
| `--ir`    | IR in readable text format                                      |
| `--cfg`   | CFG representation in the [dot](https://graphviz.org/) language |
| `--llvm`  | LLVM assembly code                                              |
| `--bc`    | LLVM bitcode (binary output)                                    |
| `--asm`   | Assmbly code (only in Unix-based systems)                       |

Each of listed options can accept optional argument with path to file (e.g. `--llvm=./out.ll`).
In this case output will be written into specified file.
If path is empty or `-`, output will be written into stdout.
Without any of these options, compiler will print nothing but errors.

### Compilation options

| Option              | Description                                          |
|---------------------|------------------------------------------------------|
| `-D <macro>`        | Define a macro with optional value                   |
| `-O <lvl>`          | Set optimization level (0-2, default is 2)           |
| `--no-s1`           | Disable special optimization 1 (intrinsics detector) |
| `--no-s2`           | ~~Disable special optimization 2 (fixpoints)~~       |
| `--llc-args <args>` | Specify arguments for llc program                    |

There are following optimization levels:

- 0: no optimizations
- 1: most common optimizations (without loops optimizations)
- 2: all available optimizations

Note, that specifying any llc argument disables default arguments: `-O0 -mcpu=native`.
There are some llc arguments, that can be interesting:

- `-march=<arch>`: specify destination architecture (e.g. `x86-64`)
- `-mcpu=<cpu>`: specify destination CPU (e.g. `native`, `skylake`)
- `-O<lvl>`: Set LLC optimization level

### Debug options

| Option         | Description                     |
|----------------|---------------------------------|
| `--times`      | Print elapsed time of each step |
| `--tr-scanner` | Enable scanner debug mode       |
| `--tr-parser`  | Enable parser debug mode        |


### External programs customization

This compiler uses some external programs via fork-exec calls.
This behaviour available only on Unix-based systems and disabled for other ones.

One can specify names for used executables via environment variables listed below.

- `CW39_LLC` - name of the llc program from LLVM toolchain (default: `llc`)

For example:

```sh
export CW39_LLC=llc-13
cw39 --asm ./test.c
# Or
CW39_LLC=llc-13 cw39 --asm ./test.c
```

## Examples

Print LLVM code into the terminal:
```sh
cw39 --llvm ./test.c
```

Print LLVM code into the `out.ll` file:
```sh
cw39 --llvm=out.ll ./test.c
```

Execute generated code with arguments 1 and 2 immediately (also `lli-13` and others can be used):
```sh
cw39 --llvm ./test.c | lli - 1 2
```

Draw CFG into the `graph.svg` file:
```sh
cw39 --cfg ./test.c | dot -Tsvg -o graph.svg
```

Create executable file from assemly code via clang.
```sh
cw39 --asm=test.s ./test.c
clang ./test.s
# Or
cw39 --asm ./test.c | clang -x assembler -
# Or
cw39 --llvm ./test.c | clang -x ir -
```

Create executable with LLVM optimizations.
```sh
cw39 --asm --llc-args="-O3 -mcpu=native" test.c | clang-10 -x assembler -
```
