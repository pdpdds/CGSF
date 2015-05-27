cmake_minimum_required(VERSION 3.2)
project(BaseLayer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library (BaseLayer SFBitArray.cpp SFBitArray.h)


