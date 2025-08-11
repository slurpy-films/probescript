cmake_minimum_required(VERSION 3.10)
project(probescript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PROBESCRIPT_EMBEDDED "If ON, probescript will be compiled to a static library" OFF)

set(
    PROBESCRIPT_INCLUDE_DIRS

    src/cli
    src/standard_lib
    src
    ${CMAKE_SOURCE_DIR}
    core
)

include_directories(${PROBESCRIPT_INCLUDE_DIRS})

if (PROBESCRIPT_EMBEDDED)
    file(GLOB_RECURSE PROBESCRIPT_SOURCES "src/standard_lib/*.cpp")
    string(APPEND PROBESCRIPT_SOURCES "src/probescript.cpp")
    add_subdirectory(core)
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
    endif()

    set(PROBESCRIPT_LIBS probescript-core)

    if(WIN32 OR MINGW OR MSYS OR CYGWIN)
        list(APPEND PROBESCRIPT_LIBS ws2_32)
    endif()

    add_library(probescript ${PROBESCRIPT_SOURCES})
    list(APPEND PROBESCRIPT_LIBS probescript)
else()
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
endif()