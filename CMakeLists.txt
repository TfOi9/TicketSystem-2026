cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

include_directories(include)

add_executable(
	code
	src/command/command.cpp src/command/token.cpp
	src/system/order.cpp src/system/ticket.cpp src/system/train.cpp src/system/user.cpp
	src/utils/time_date.cpp src/utils/validator.cpp
	src/main.cpp
)

add_executable(bpt src/bpt.cpp)

add_executable(cleanup src/cleanup.cpp)

add_executable (
	server
	frontend/src/web/server.cpp
	frontend/include/web/tcpserver.hpp
	frontend/src/web/tcpserver.cpp
)

add_executable (
	client
	frontend/src/web/client.cpp
	frontend/include/web/tcpclient.hpp
	frontend/src/web/tcpclient.cpp
)

target_link_libraries(server Qt6::Core Qt6::Widgets Qt6::Network)

target_link_libraries(client Qt6::Core Qt6::Widgets Qt6::Network)