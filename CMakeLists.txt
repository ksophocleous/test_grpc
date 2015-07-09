cmake_minimum_required(VERSION 2.8.4)

project(grpc_cpp)

if(MSVC)
	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
	endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND src
	"main.cpp"
	"test.grpc.pb.cc"
	"test.pb.cc"
	"test.grpc.pb.h"
	"test.pb.h"
	"test.proto")

list (APPEND incdirs
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/protobuf/src"
	)

add_executable(main ${src})
set_target_properties(main PROPERTIES
	INCLUDE_DIRECTORIES "${incdirs}")

target_link_libraries(main
	"${CMAKE_CURRENT_SOURCE_DIR}/gpr.lib"
	"${CMAKE_CURRENT_SOURCE_DIR}/libprotobuf.lib"
	"${CMAKE_CURRENT_SOURCE_DIR}/grpc.lib"
	"${CMAKE_CURRENT_SOURCE_DIR}/grpc++.lib"
	"ws2_32.lib"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../packages/grpc.dependencies.zlib.1.2.8.9/build/native/lib/v120/x64/Release/dynamic/stdcall/zlib.lib")

add_custom_command(OUTPUT "test.grpc.pb.h" "test.grpc.pb.cc"
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/protoc.exe" --grpc_out=. --plugin=protoc-gen-grpc="${CMAKE_CURRENT_SOURCE_DIR}/grpc_cpp_plugin.exe" test.proto
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Generating grpc protobuf souce and header")

add_custom_command(OUTPUT "test.pb.h" "test.pb.cc"
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/protoc.exe" --cpp_out=. test.proto
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Generating protobuf souce and header")
