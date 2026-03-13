cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

include_directories(include)
include_directories(frontend/include)

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
	frontend/include/web/tcp/tcpserver.hpp
	frontend/src/web/tcp/tcpserver.cpp
	frontend/include/web/udp/udpserver.hpp
	frontend/src/web/udp/udpserver.cpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/include/web/dispatch/sender.hpp
	frontend/src/web/dispatch/dispatcher.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/tlv/tlvparser.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/tlv/tlvparser.cpp
	src/command/command.cpp src/command/token.cpp
	src/system/order.cpp src/system/ticket.cpp src/system/train.cpp src/system/user.cpp
	src/utils/time_date.cpp src/utils/validator.cpp
)

add_executable (
	client
	frontend/src/web/client.cpp
	frontend/include/web/tcp/tcpclient.hpp
	frontend/src/web/tcp/tcpclient.cpp
	frontend/include/web/udp/udpclient.hpp
	frontend/src/web/udp/udpclient.cpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/include/web/dispatch/sender.hpp
	frontend/src/web/dispatch/dispatcher.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/tlv/tlvparser.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/tlv/tlvparser.cpp
	src/command/command.cpp src/command/token.cpp
	src/system/order.cpp src/system/ticket.cpp src/system/train.cpp src/system/user.cpp
	src/utils/time_date.cpp src/utils/validator.cpp
)

target_link_libraries(server Qt6::Core Qt6::Widgets Qt6::Network)

target_link_libraries(client Qt6::Core Qt6::Widgets Qt6::Network)

add_executable(tlvpacket_test
	test/tlv/tlvpacket_test.cpp
	frontend/src/web/tlv/tlvpacket.cpp
)
target_link_libraries(tlvpacket_test Qt6::Core)

add_executable(tlvparser_test
	test/tlv/tlvparser_test.cpp
	frontend/include/web/tlv/tlvparser.hpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/src/web/tlv/tlvparser.cpp
	frontend/src/web/tlv/tlvpacket.cpp
)
target_link_libraries(tlvparser_test Qt6::Core)

add_executable(dispatcher_test
	test/tlv/dispatcher_test.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/dispatch/dispatcher.cpp
)
target_link_libraries(dispatcher_test Qt6::Core)

add_executable(tcp_test
	test/tlv/tcp_test.cpp
	frontend/include/web/tcp/tcpserver.hpp
	frontend/include/web/tcp/tcpclient.hpp
	frontend/src/web/tcp/tcpserver.cpp
	frontend/src/web/tcp/tcpclient.cpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/include/web/dispatch/sender.hpp
	frontend/src/web/dispatch/dispatcher.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/tlv/tlvparser.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/tlv/tlvparser.cpp
)
target_link_libraries(tcp_test Qt6::Core Qt6::Network)

add_executable(tcp_tlv_mixed_test
	test/tlv/tcp_tlv_mixed_test.cpp
	frontend/include/web/tcp/tcpserver.hpp
	frontend/include/web/tcp/tcpclient.hpp
	frontend/src/web/tcp/tcpserver.cpp
	frontend/src/web/tcp/tcpclient.cpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/include/web/dispatch/sender.hpp
	frontend/src/web/dispatch/dispatcher.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/tlv/tlvparser.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/tlv/tlvparser.cpp
)
target_link_libraries(tcp_tlv_mixed_test Qt6::Core Qt6::Network)

add_executable(dispatcher_large_test
	test/tlv/dispatcher_large_test.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/dispatch/dispatcher.cpp
)
target_link_libraries(dispatcher_large_test Qt6::Core)

add_executable(tcp_robustness_test
	test/tlv/tcp_robustness_test.cpp
	frontend/include/web/tcp/tcpserver.hpp
	frontend/include/web/tcp/tcpclient.hpp
	frontend/src/web/tcp/tcpserver.cpp
	frontend/src/web/tcp/tcpclient.cpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/include/web/dispatch/sender.hpp
	frontend/src/web/dispatch/dispatcher.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/tlv/tlvparser.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/tlv/tlvparser.cpp
)
target_link_libraries(tcp_robustness_test Qt6::Core Qt6::Network)

add_executable(tcp_large_robustness_test
	test/tlv/tcp_large_robustness_test.cpp
	frontend/include/web/tcp/tcpserver.hpp
	frontend/include/web/tcp/tcpclient.hpp
	frontend/src/web/tcp/tcpserver.cpp
	frontend/src/web/tcp/tcpclient.cpp
	frontend/include/web/dispatch/dispatcher.hpp
	frontend/src/web/dispatch/dispatcher.cpp
	frontend/include/web/tlv/tlvpacket.hpp
	frontend/include/web/tlv/tlvparser.hpp
	frontend/src/web/tlv/tlvpacket.cpp
	frontend/src/web/tlv/tlvparser.cpp
)
target_link_libraries(tcp_large_robustness_test Qt6::Core Qt6::Network)

add_executable(fixed_string_test
	test/fixed_string_test.cpp
)

add_executable(type_helper_test
	test/type_helper_test.cpp
)

add_test(NAME fixed_string_test COMMAND fixed_string_test)
add_test(NAME type_helper_test COMMAND type_helper_test)
add_test(NAME tlvpacket_test COMMAND tlvpacket_test)
add_test(NAME tlvparser_test COMMAND tlvparser_test)
add_test(NAME dispatcher_test COMMAND dispatcher_test)
add_test(NAME dispatcher_large_test COMMAND dispatcher_large_test)
add_test(NAME tcp_test COMMAND tcp_test)
add_test(NAME tcp_tlv_mixed_test COMMAND tcp_tlv_mixed_test)
add_test(NAME tcp_robustness_test COMMAND tcp_robustness_test)
add_test(NAME tcp_large_robustness_test COMMAND tcp_large_robustness_test)

# WARNING: this test is very large, may take up to 5min or more!
# add_test(
# 	NAME ticket_full_regression
# 	COMMAND bash ${CMAKE_SOURCE_DIR}/tools/test.sh
# )