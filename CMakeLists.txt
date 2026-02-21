cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories(include)

# add_executable(code src/main.cpp)

add_executable(code src/bpt.cpp)

add_executable(cleanup src/cleanup.cpp)

enable_testing()

add_test(
	NAME storage_test_1
	COMMAND /bin/sh -c "\"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in1.txt\" > \"${CMAKE_BINARY_DIR}/storage_out1.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans1.txt\" \"${CMAKE_BINARY_DIR}/storage_out1.txt\" && \"$<TARGET_FILE:cleanup>\""
)

add_test(
	NAME storage_test_2
	COMMAND /bin/sh -c "\"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in2.txt\" > \"${CMAKE_BINARY_DIR}/storage_out2.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans2.txt\" \"${CMAKE_BINARY_DIR}/storage_out2.txt\" && \"$<TARGET_FILE:cleanup>\""
)

add_test(
	NAME storage_test_3
	COMMAND /bin/sh -c "\"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in3.txt\" > \"${CMAKE_BINARY_DIR}/storage_out3.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans3.txt\" \"${CMAKE_BINARY_DIR}/storage_out3.txt\" && \"$<TARGET_FILE:cleanup>\""
)

add_test(
	NAME storage_test_4
	COMMAND /bin/sh -c "\"$<TARGET_FILE:cleanup>\" && \"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in4_round1.txt\" > \"${CMAKE_BINARY_DIR}/storage_out4_round1.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans4_round1.txt\" \"${CMAKE_BINARY_DIR}/storage_out4_round1.txt\" && \"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in4_round2.txt\" > \"${CMAKE_BINARY_DIR}/storage_out4_round2.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans4_round2.txt\" \"${CMAKE_BINARY_DIR}/storage_out4_round2.txt\" && \"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in4_round3.txt\" > \"${CMAKE_BINARY_DIR}/storage_out4_round3.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans4_round3.txt\" \"${CMAKE_BINARY_DIR}/storage_out4_round3.txt\" && \"$<TARGET_FILE:cleanup>\""
)

add_test(
	NAME storage_test_5
	COMMAND /bin/sh -c "\"$<TARGET_FILE:cleanup>\" && \"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in5_round1.txt\" > \"${CMAKE_BINARY_DIR}/storage_out5_round1.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans5_round1.txt\" \"${CMAKE_BINARY_DIR}/storage_out5_round1.txt\" && \"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in5_round2.txt\" > \"${CMAKE_BINARY_DIR}/storage_out5_round2.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans5_round2.txt\" \"${CMAKE_BINARY_DIR}/storage_out5_round2.txt\" && \"$<TARGET_FILE:code>\" < \"${CMAKE_SOURCE_DIR}/test/storage/in5_round3.txt\" > \"${CMAKE_BINARY_DIR}/storage_out5_round3.txt\" && diff -u \"${CMAKE_SOURCE_DIR}/test/storage/ans5_round3.txt\" \"${CMAKE_BINARY_DIR}/storage_out5_round3.txt\" && \"$<TARGET_FILE:cleanup>\""
)

set_tests_properties(storage_test_1 storage_test_2 storage_test_3 storage_test_4 storage_test_5 PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(storage_test_2 PROPERTIES DEPENDS storage_test_1)
set_tests_properties(storage_test_3 PROPERTIES DEPENDS storage_test_2)
set_tests_properties(storage_test_4 PROPERTIES DEPENDS storage_test_3)
set_tests_properties(storage_test_5 PROPERTIES DEPENDS storage_test_4)