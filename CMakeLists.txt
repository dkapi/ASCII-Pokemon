cmake_minimum_required(VERSION 3.16)
project(ASCII-Pokemon C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
# GNU extensions are left enabled (the default) because heap.c uses
# statement-expressions and typeof.

add_executable(worldgen
  worldgen.c
  voronoi.c
  heap.c
  dijkstra.c
  characters.c
  tiles.c
)

target_compile_options(worldgen PRIVATE -Wall -Wextra)