cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories(include)

# add_executable(code src/main.cpp)

add_executable(code src/bpt.cpp)

add_executable(cleanup src/cleanup.cpp)