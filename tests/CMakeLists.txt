# Undo Library
# Copyright (C) 2015-2021 David Capello

function(add_undo_test name)
  add_executable(${name} ${name}.cpp)
  add_test(NAME ${name} COMMAND ${name})
  target_link_libraries(${name} undo)
  target_include_directories(${name} PUBLIC ..)
endfunction()

add_undo_test(basics)
add_undo_test(complex_tree)
add_undo_test(tree)
