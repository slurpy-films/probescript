cmake_minimum_required(VERSION 3.10)
project(probescript-core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

file(GLOB_RECURSE CORE_SOURCES *.cpp **/*.cpp)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

add_library(probescript-core ${CORE_SOURCES})