cmake_minimum_required(VERSION 3.17)
project(CScripty C)

set(CMAKE_C_STANDARD 90)

add_executable(CScripty src/main.c src/common.h src/chunk.c src/chunk.h src/memory.c src/memory.h)