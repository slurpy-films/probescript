# probescript-core
# This module includes the parser, interpreter and typechecker of probescript

cmake_minimum_required(VERSION 3.10)
project(probescript-core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/core)

file(GLOB_RECURSE CORE_SOURCES "*.cpp" "**/*.cpp")

add_library(probescript-core ${CORE_SOURCES})