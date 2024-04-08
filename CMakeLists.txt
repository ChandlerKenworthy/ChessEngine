cmake_minimum_required(VERSION 3.20)
project(ChessEngine VERSION 0.1.0)

cmake_policy(SET CMP0148 NEW)

# Set Qt6 installation path
set(Qt6_DIR "/Users/chandler/Qt/6.6.2/macos/lib/cmake/Qt6/")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)

# Set Python executable path
#if(DEFINED ENV{PYTHON_EXECUTABLE})
#    set(PYTHON_EXECUTABLE $ENV{PYTHON_EXECUTABLE})
#else()
#    set(PYTHON_EXECUTABLE "/Users/chandler/miniconda/envs/tf/bin/python")
#endif()

if(APPLE)
    set(PYTHON_EXECUTABLE "/Users/chandler/miniconda/envs/tf/bin/python")
endif()

# Set pybind11 directory
if(APPLE)
    set(pybind11_DIR "/Users/chandler/miniconda/lib/python3.10/site-packages/pybind11/share/cmake/pybind11")
endif()
#if(DEFINED ENV{PYBIND11_DIR})
#    set(pybind11_DIR $ENV{PYBIND11_DIR})
#else()
#    set(pybind11_DIR "/Users/chandler/miniconda/lib/python3.10/site-packages/pybind11/share/cmake/pybind11")
#endif()

# Find Qt6 package
find_package(Qt6 COMPONENTS Widgets REQUIRED)
#find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Add your source files
add_executable(ChessEngine 
    main.cpp
    src/Board.cpp
    src/Engine.cpp
    src/Generator.cpp
    src/Renderer.cpp
    src/Test.cpp
)

add_subdirectory(extern/pybind11)

# Include SFML headers
target_include_directories(ChessEngine PRIVATE 
    src 
    include
    ${Qt6Widgets_INCLUDE_DIRS}
)

# Use MOC to process header files containing Q_OBJECT
qt6_wrap_cpp(MOC_SOURCES include/Renderer.hpp)

# Add generated MOC files to the target
target_sources(ChessEngine PRIVATE ${MOC_SOURCES})

# Add your source files for the Python module
pybind11_add_module(chess_engine
    python_bindings/py_generator.cpp
    src/Board.cpp
    src/Generator.cpp
    src/Engine.cpp
)

# Include directories for the Python module
target_include_directories(chess_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
)

# Include directories for the Python module
target_include_directories(chess_engine PRIVATE 
    src 
    include
)

# Link against SFML libraries
target_link_libraries(ChessEngine PRIVATE 
    Qt6::Widgets
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror -Wfatal-errors -Wshadow -pedantic")