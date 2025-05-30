cmake_minimum_required(VERSION 3.10)
project(probescript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

add_executable(probescript ${SOURCES})

if(WIN32 OR MINGW OR MSYS OR CYGWIN)
    target_link_libraries(probescript ws2_32)
endif()