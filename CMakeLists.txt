cmake_minimum_required(VERSION 3.10)
project(probescript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    src/cli
    src/standard_lib
    ${CMAKE_SOURCE_DIR}
    core
)

file(GLOB_RECURSE MAIN_SOURCES "src/cli/*.cpp" "src/standard_lib/*.cpp")

add_subdirectory(core)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

add_executable(probescript ${MAIN_SOURCES})

set(PROBESCRIPT_TARGET_LIBS probescript-core)

if(WIN32 OR MINGW OR MSYS OR CYGWIN)
    list(APPEND PROBESCRIPT_TARGET_LIBS ws2_32)
endif()

target_link_libraries(probescript PRIVATE ${PROBESCRIPT_TARGET_LIBS})