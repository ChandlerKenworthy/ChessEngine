cmake_minimum_required(VERSION 3.2)
project(ChessEngine VERSION 0.1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

add_executable(ChessEngine 
    Game.cpp
    src/Board.cpp
    src/Engine.cpp
)

target_include_directories(ChessEngine
    PRIVATE src include
)

# set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wfatal-errors -Wshadow -pedantic")