cmake_minimum_required(VERSION 3.10)

# Set the compiling mode flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -s")

# Set the project name
project(air_control_c)

# Include the current directory for header files
include_directories(include)

# Include additional libraries

# Add the executable
add_executable(air_control_c
 src/main.c
 src/functions.c
)