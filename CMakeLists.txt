cmake_minimum_required(VERSION 3.15)
project(gt9271_driver VERSION 1.0.0 LANGUAGES C)

# 1. Strict Compiler Flags (The "Catching Errors" part)
# -Wall -Wextra: Enable almost all warnings
# -Wpedantic: Ensure strict ISO C compliance
# -Wshadow: Warn if a variable shadows another
# -Werror: Treat warnings as errors (Force you to fix them)
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Werror)
endif()

# 2. Define the Library
# We use STATIC because embedded projects usually link everything into one binary
add_library(gt9271 STATIC 
    src/ctp.c
)

# 3. Include Directories
# PUBLIC means anyone who links to this library also gets these include paths
target_include_directories(gt9271 
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 4. Embedded Specifics (Optional but recommended)
# This prevents the compiler from assuming a standard OS environment exists
set_target_properties(gt9271 PROPERTIES 
    C_STANDARD 11 
    C_STANDARD_REQUIRED ON
)
