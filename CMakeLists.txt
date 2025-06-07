cmake_minimum_required(VERSION 3.10)
project(probescript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src/core src/cli src/standard_lib)

file(GLOB_RECURSE MAIN_SOURCES "src/cli/*.cpp" "src/core/*.cpp" "src/standard_lib/*.cpp")
file(GLOB_RECURSE INSTALLER_SOURCES "src/installer/*.cpp")

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

add_executable(probescript ${MAIN_SOURCES})
add_executable(installer ${INSTALLER_SOURCES})

if(WIN32 OR MINGW OR MSYS OR CYGWIN)
    target_link_libraries(probescript ws2_32)
endif()