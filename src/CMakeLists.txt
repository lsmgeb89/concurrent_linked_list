cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(concurrent_linked_list VERSION 1.0 LANGUAGES CXX C)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_executable(concurrent_linked_list
  main.cc
  utils/coarse_grained_linked_list.h
  utils/fine_grained_linked_list.h
  utils/lock_free_linked_list.h
  utils/list_node.h)

target_include_directories(concurrent_linked_list PRIVATE utils)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(concurrent_linked_list PRIVATE -std=c++0x)
  target_link_libraries(concurrent_linked_list PRIVATE Threads::Threads)
endif()
