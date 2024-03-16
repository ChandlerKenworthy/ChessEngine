cmake_minimum_required(VERSION 3.20)
project(ChessEngine VERSION 0.1.0)

# Set Qt6 installation path
set(Qt6_DIR "/Users/chandler/Qt/6.6.2/macos/lib/cmake/Qt6/")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

# Find Qt6 package
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Add your source files
add_executable(ChessEngine 
    main.cpp
    src/Board.cpp
    src/Engine.cpp
    src/Generator.cpp
    src/Renderer.cpp
    src/Test.cpp
)

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

# Link against SFML libraries
target_link_libraries(ChessEngine PRIVATE 
    Qt6::Widgets
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror -Wfatal-errors -Wshadow -pedantic")