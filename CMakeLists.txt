cmake_minimum_required(VERSION 3.23)
project(evol)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")

add_executable(evol main.cpp market_strategy/strategy.cpp market_strategy/states.cpp deal_tracker/deal_tracker.cpp limit_strategy/strategy.cpp limit_strategy/strategy.hpp limit_strategy/states.cpp limit_strategy/states.hpp)
