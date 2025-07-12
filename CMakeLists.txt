cmake_minimum_required(VERSION 3.10)
project(probescript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static linking of LLVM
set(LLVM_USE_STATIC_LIBS ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Add probescript core library
add_subdirectory(core)

# Include local project directories
include_directories(
    src/cli
    src/standard_lib
    src/compiler
    src
    ${CMAKE_SOURCE_DIR} # For including core/
    core # We still use core/ for core library includes in implementation
)

# Collect all CLI/main sources
file(GLOB_RECURSE MAIN_SOURCES
    src/cli/*.cpp
    src/standard_lib/*.cpp
    src/compiler/*.cpp
)

# Define the main executable
add_executable(probescript ${MAIN_SOURCES})

# Define which libraries to link with
set(PROBESCRIPT_STATIC_LIBS
    probescript-core

    # LLVM core libraries
    LLVMCore
    LLVMSupport
    LLVMIRReader
    LLVMAnalysis
    LLVMPasses

    # AArch64
    LLVMAArch64CodeGen
    LLVMAArch64AsmParser
    LLVMAArch64Desc
    LLVMAArch64Disassembler
    LLVMAArch64Info
    LLVMAArch64Utils

    # ARM
    LLVMARMCodeGen
    LLVMARMAsmParser
    LLVMARMDesc
    LLVMARMDisassembler
    LLVMARMInfo
    LLVMARMUtils

    # X86
    LLVMX86CodeGen
    LLVMX86AsmParser
    LLVMX86Desc
    LLVMX86Disassembler
    LLVMX86Info
)

# Link winsock2 for the http standard library
if(WIN32 OR MINGW OR MSYS OR CYGWIN)
    list(APPEND PROBESCRIPT_STATIC_LIBS ws2_32)
endif()

# Link the executable
target_link_libraries(probescript PRIVATE ${PROBESCRIPT_STATIC_LIBS})