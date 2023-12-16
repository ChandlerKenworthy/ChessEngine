# Chess Engine

This is a personal project where I am aiming to build a chess engine (e.g. a chess "bot") that is capable of playing good quality moves against a human or AI opponent. My aims for this project are:

- To develop a chess engine in C++ using a bitboard approach that is capable of consistently beating opponents at the 1000 elo level or higher.
- Learn and implement modern C++ such as using smart pointers for better memory management, proper unit testing and writing efficienct code leveraging std::algorithm etc.
- Potentially use some of my machine learning background and knowledge to inform move making decisions.

## How to Use
See "installation" instructions below:
1. Download or clone this repository
2. Inside the "ChessEngine" directory make a new directory "build"
3. cd build
4. cmake ../src
5. build [-j 4]
6. The run the program with ./ChessEngine

Only tested with CMake version 3.27.9 and Apple clang version 15.0.0 (clang-1500.0.40.1)
