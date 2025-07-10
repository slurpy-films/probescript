cmake_minimum_required(VERSION 3.10)
project(probescript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
include_directories(src/core src/cli src/standard_lib src)
include_directories(${LLVM_INCLUDE_DIRS})

set(LLVM_USE_STATIC_LIBS ON)

file(GLOB_RECURSE MAIN_SOURCES "src/cli/*.cpp" "src/core/*.cpp" "src/standard_lib/*.cpp" src/compiler/*.cpp)

add_executable(probescript ${MAIN_SOURCES})

target_link_libraries(probescript PRIVATE
    LLVMCore
    LLVMSupport
    LLVMIRReader
    LLVMMCJIT
    LLVMNativeCodeGen
    LLVMAnalysis
    LLVMPasses
)

if(WIN32 OR MINGW OR MSYS OR CYGWIN)
    target_link_libraries(probescript ws2_32)
endif()