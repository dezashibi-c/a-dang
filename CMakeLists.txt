###############################################################################
# GENERAL CONFIGURATIONS AND UTILITIES
###############################################################################
cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 11)
include(cmake/utils.cmake)

###############################################################################
# PROJECT SETUP
###############################################################################
project(DangCompiler LANGUAGES C)
include_directories(include src extern)
include(cmake/configs.cmake)
add_subdirectory(tests)

###############################################################################
# EXECUTABLE TARGETS
###############################################################################
add_executable(dang 
    src/main.c
    src/common.c
    src/scanner.c
    src/token.c
    src/ast.c
    src/parser.c
    src/evaluator.c
)

# define_macro_option(dang PRINT_GREETINGS ON)

###############################################################################
# PRE BUILD TESTS
###############################################################################

enable_pre_build_testing(dang all_tests)

###############################################################################
# POST BUILD FILE COPIES
###############################################################################
# set(FILES_TO_COPY
#     "${CMAKE_SOURCE_DIR}/LICENSE"
# )

# define_post_built_copy(dang "" ${FILES_TO_COPY})
