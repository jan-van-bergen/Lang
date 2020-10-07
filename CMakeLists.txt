cmake_minimum_required (VERSION 3.3)
project(Lang)

file(GLOB src *.h *.c)

add_executable(Lang ${src})
