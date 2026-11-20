cmake_minimum_required(VERSION 3.15)
project(gt9271_driver LANGUAGES C)

#  Use Strict Compiler Flags 
# -Wall -Wextra: Enable almost all warnings
# -Wpedantic: Ensure strict ISO C compliance
# -Wshadow: Warn if a variable shadows another
# -Werror: Treat warnings as errors (Force you to fix them)
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Werror)
endif()

# Define the Library
add_library(gt9271 STATIC 
    src/ctp.c
    src/ctp_hal.c
    src/ctp_touch.c
    src/ctp_default_config.c
)

# Include Directories
include_directories(${CMAKE_BINARY_DIR}/generated)
target_include_directories(gt9271 
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Embedded Specifics 
set_target_properties(gt9271 PROPERTIES 
    C_STANDARD 11 
    C_STANDARD_REQUIRED ON
)
