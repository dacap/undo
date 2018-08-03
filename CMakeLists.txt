# Undo Library
# Copyright (C) 2015-2018 David Capello

cmake_minimum_required(VERSION 3.2)

project(undo CXX)
option(UNDO_TESTS "Compile undo tests" on)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

add_library(undo undo_history.cpp)

if(UNDO_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
