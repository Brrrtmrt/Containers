cmake_minimum_required(VERSION 4.0)
project(untitled1)

set(CMAKE_CXX_STANDARD 23)

add_library(untitled1 STATIC
        Iterator.h
        testmain.cpp
        SkipNode.h
)
