# Dang Compiler

The idea behind `dang` is to implement a simple programming language which:

- Everything is statement
- ...

## Structure

- `cmake` any utility and re-usable files for your cmakes goes here.
- `extern` folder is where you put your external dependencies, as an example I have used `clove-unit` library for unit testing.
- `include` is where you put your exported header files.
- `src` is where your `.c` files must be placed.
- `tests` are written using `clove-unit` framework which is a lightweight and single header library.

**👉 NOTE:** You can refer to [here](https://github.com/fdefelici/clove-unit) for more information about `clove-unit`.

**👉 NOTE:** There is also `.clang-format` available for you to use.

**👉 NOTE:** All the build artifacts will be placed in `out` folder, all the build artifacts for tests will be placed in `out_tests` folder.

## CMake Configuration Options

- `dang_ENABLE_NAN_BOXING` -> `ON` by default
- `dang_ENABLE_DEBUG_PRINT_CODE` -> `OFF` by default
- `dang_ENABLE_DEBUG_TRACE_EXECUTION` -> `OFF` by default
- `dang_ENABLE_DEBUG_STRESS_GC` -> `ON` by default
- `dang_ENABLE_DEBUG_LOG_GC` -> `OFF` by default

- `dang_dbg_ENABLE_NAN_BOXING` -> `ON` by default
- `dang_dbg_ENABLE_DEBUG_PRINT_CODE` -> `ON` by default
- `dang_dbg_ENABLE_DEBUG_TRACE_EXECUTION` -> `ON` by default
- `dang_dbg_ENABLE_DEBUG_STRESS_GC` -> `ON` by default
- `dang_dbg_ENABLE_DEBUG_LOG_GC` -> `OFF` by default

## License

Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0) License.

Copyright (c) 2024, Navid Dezashibi <navid@dezashibi.com>

Please refer to [LICENSE](/LICENSE) file.
