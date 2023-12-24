cmake_minimum_required(VERSION 3.2)
project(ChessEngine VERSION 0.1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

# Find SFML
find_package(SFML COMPONENTS graphics window system REQUIRED)

# Add your source files
add_executable(ChessEngine 
    main.cpp
    #src/Game.cpp
    src/Board.cpp
    src/Engine.cpp
    src/Renderer.cpp
)

# Include SFML headers
target_include_directories(ChessEngine PRIVATE 
    src 
    include
    ${SFML_INCLUDE_DIR}  # This line includes SFML headers
)

# Link against SFML libraries
target_link_libraries(ChessEngine PRIVATE 
    sfml-graphics 
    sfml-window 
    sfml-system
)

# set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wfatal-errors -Wshadow -pedantic")