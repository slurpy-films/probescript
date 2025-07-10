cmake_minimum_required(VERSION 3.10)
project(probescript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(src/core src/cli src/standard_lib src)
include_directories(${LLVM_INCLUDE_DIRS})

set(LLVM_USE_STATIC_LIBS ON)

file(GLOB_RECURSE MAIN_SOURCES "src/cli/*.cpp" "src/core/*.cpp" "src/standard_lib/*.cpp" src/compiler/*.cpp)

add_executable(probescript ${MAIN_SOURCES})

set(PROBESCRIPT_STATIC_LIBS
    LLVMCore
    LLVMSupport
    LLVMIRReader
    LLVMMCJIT
    LLVMAnalysis
    LLVMPasses

    LLVMAArch64CodeGen
    LLVMAArch64AsmParser
    LLVMAArch64Desc
    LLVMAArch64Disassembler
    LLVMAArch64Info
    LLVMAArch64Utils

    LLVMARMCodeGen
    LLVMARMAsmParser
    LLVMARMCodeGen
    LLVMARMDesc
    LLVMARMDisassembler
    LLVMARMInfo
    LLVMARMUtils

    LLVMX86CodeGen
    LLVMX86AsmParser
    LLVMX86Desc
    LLVMX86Disassembler
    LLVMX86Info
)

if(WIN32 OR MINGW OR MSYS OR CYGWIN)
    list(APPEND PROBESCRIPT_STATIC_LIBS ws2_32)
endif()

target_link_libraries(probescript PRIVATE ${PROBESCRIPT_STATIC_LIBS})