cmake_minimum_required (VERSION 3.3)
project(Lang)

file(GLOB src Src/*.h Src/*.c)

add_executable(Lang ${src})
