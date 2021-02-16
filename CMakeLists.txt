cmake_minimum_required(VERSION 3.17)
project(primes)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.75.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(primes main.cpp)
endif()

set(CMAKE_CXX_FLAGS "-lboost_program_options")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(primes PRIVATE Threads::Threads)
