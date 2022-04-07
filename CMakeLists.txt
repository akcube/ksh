cmake_minimum_required(VERSION 3.22.3)
project(KSH VERSION 1.0 DESCRIPTION "A Shell written in C" LANGUAGES C)

set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall -O0 -g -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE_INIT "-O3 -ffast-math -march=native")

add_subdirectory(src)
