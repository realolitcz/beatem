cmake_minimum_required(VERSION 4.0)
project(beatem)

set(CMAKE_CXX_STANDARD 23)

add_executable(beatem
        game.cpp
        window.cpp
        window.h
        defines.h
        logic.cpp
        logic.h
        window.cpp
)
