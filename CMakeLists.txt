cmake_minimum_required( VERSION 3.0 )

project( mazesolzing-cpp )

# flags

# include files
include_directories( ./include ./src )

# target
add_executable( mazesolver ./src/main.cpp )

# 3rd party libs