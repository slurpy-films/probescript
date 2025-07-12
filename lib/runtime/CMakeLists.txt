cmake_minimum_required(VERSION 3.10)
project(probescript-runtime)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_library(probescript-runtime STATIC ${SOURCES})

target_compile_options(probescript-runtime PRIVATE)