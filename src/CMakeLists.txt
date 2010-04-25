#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.6.2)
project (songbook)

SET(CMAKE_CXX_FLAGS_RELEASE "-DINLINE_FLAG -O2 -finline-functions -DNDEBUG -march=native")

#-------------------------------------------------------------------------------
add_subdirectory (ihm)
#-------------------------------------------------------------------------------
