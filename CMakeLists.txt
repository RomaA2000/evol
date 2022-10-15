cmake_minimum_required(VERSION 3.23)
project(evol)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")

add_executable(evol main.cpp strategy/strategy.cpp strategy/states.cpp deal_tracker/deal_tracker.cpp)
