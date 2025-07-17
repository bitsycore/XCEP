cmake_minimum_required(VERSION 3.22)
project(Exceptions C)

set(CMAKE_C_STANDARD 99)

add_executable(Exceptions
        main.c
        Exceptions.c
        Exceptions.h
)