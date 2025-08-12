cmake_minimum_required(VERSION 3.10)
project(probescript)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PROBESCRIPT_EMBEDDED "If ON, probescript will be compiled to a static library" OFF)

set(PROBESCRIPT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli
    ${CMAKE_CURRENT_SOURCE_DIR}/core/standard_lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}
)

include_directories(${PROBESCRIPT_INCLUDE_DIRS})

set(PROBESCRIPT_INCLUDE_DIRS ${PROBESCRIPT_INCLUDE_DIRS} PARENT_SCOPE)

message(STATUS ${PROBESCRIPT_EMBEDDED})

if (PROBESCRIPT_EMBEDDED)
    file(GLOB_RECURSE PROBESCRIPT_SOURCES "src/probescript.cpp")
    add_subdirectory(core)
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
    endif()

    set(PROBESCRIPT_LIBS probescript-core)

    if(WIN32 OR MINGW OR MSYS OR CYGWIN)
        list(APPEND PROBESCRIPT_LIBS ws2_32)
    endif()

    add_library(probescript STATIC ${PROBESCRIPT_SOURCES})
    target_link_libraries(probescript ${PROBESCRIPT_LIBS})
else()
    file(GLOB_RECURSE MAIN_SOURCES "src/cli/*.cpp")

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