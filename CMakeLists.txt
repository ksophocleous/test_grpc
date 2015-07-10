cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0022 NEW)

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

if(MSVC)
	foreach(flag_var
		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
endif()

list(APPEND common_srcs
	"grpc_inc.h"
	"test.grpc.pb.cc"
	"test.pb.cc"
	"test.proto")

set(CONFIG "Release")

set(grpc_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../grpc")
set(protobuf_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../grpc/third_party/protobuf")
set(zlib_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/build-msvc2013/installed")
set(openssl_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../openssl-1.0.2c/installed")

configure_file("ca.cert.pem"
	"${CMAKE_CURRENT_BINARY_DIR}/bin/ca.cert.pem"
	COPYONLY)

configure_file("server.cert.pem"
	"${CMAKE_CURRENT_BINARY_DIR}/bin/server.cert.pem"
	COPYONLY)

configure_file("server.key.pem"
	"${CMAKE_CURRENT_BINARY_DIR}/bin/server.key.pem"
	COPYONLY)

configure_file("final.cert.pem"
	"${CMAKE_CURRENT_BINARY_DIR}/bin/final.cert.pem"
	COPYONLY)

add_library(gpr INTERFACE)
target_link_libraries(gpr INTERFACE
	optimized "${grpc_ROOT}/vsprojects/x64/Release/gpr.lib"
	debug "${grpc_ROOT}/vsprojects/x64/Debug/gpr.lib")
target_include_directories(gpr INTERFACE
	"${grpc_ROOT}/include")

add_library(grpc++ INTERFACE)
target_link_libraries(grpc++ INTERFACE
	optimized "${grpc_ROOT}/vsprojects/x64/Release/grpc++.lib"
	debug "${grpc_ROOT}/vsprojects/x64/Debug/grpc++.lib")
target_include_directories(grpc++ INTERFACE
	"${grpc_ROOT}/include")
	
add_library(grpc INTERFACE)
target_link_libraries(grpc INTERFACE
	optimized "${grpc_ROOT}/vsprojects/x64/Release/grpc.lib"
	debug "${grpc_ROOT}/vsprojects/x64/Debug/grpc.lib")
target_include_directories(grpc INTERFACE
	"${grpc_ROOT}/include")

add_library(protobuf INTERFACE)
target_link_libraries(protobuf INTERFACE
	optimized "${protobuf_ROOT}/vsprojects/x64/Release/libprotobuf.lib"
	debug "${protobuf_ROOT}/vsprojects/x64/Debug/libprotobuf.lib")
target_include_directories(protobuf INTERFACE
	"${protobuf_ROOT}/src")

add_library(zlib INTERFACE IMPORTED)
set_target_properties(zlib PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${zlib_ROOT}/include"
	INTERFACE_LINK_LIBRARIES "${zlib_ROOT}/lib/zlibstatic.lib")

add_library(libeay32 INTERFACE IMPORTED)
set_target_properties(libeay32 PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${openssl_ROOT}/include"
	INTERFACE_LINK_LIBRARIES "${openssl_ROOT}/lib/libeay32.lib")

add_library(ssleay32 INTERFACE IMPORTED)
set_target_properties(ssleay32 PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${openssl_ROOT}/include"
	INTERFACE_LINK_LIBRARIES "${openssl_ROOT}/lib/ssleay32.lib")

add_executable(server "server.cpp" ${common_srcs})
set_target_properties(server PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(server
	grpc
	grpc++
	gpr
	protobuf
	libeay32
	ssleay32
	zlib
	"ws2_32.lib")

add_executable(client "client.cpp" ${common_srcs})
set_target_properties(client PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(client
	grpc
	grpc++
	gpr
	protobuf
	libeay32
	ssleay32
	zlib
	"ws2_32.lib")

set_source_files_properties("test.grpc.pb.cc" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /wd4244 /wd4267 /wd4997 /wd4996")
set_source_files_properties("test.pb.cc" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /wd4244 /wd4267 /wd4997 /wd4996")

add_custom_command(OUTPUT "test.grpc.pb.h" "test.grpc.pb.cc"
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/protoc.exe" --grpc_out="${CMAKE_CURRENT_BINARY_DIR}" --plugin=protoc-gen-grpc="${CMAKE_CURRENT_SOURCE_DIR}/grpc_cpp_plugin.exe" test.proto
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Generating grpc protobuf souce and header")

add_custom_command(OUTPUT "test.pb.h" "test.pb.cc"
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/protoc.exe" --cpp_out="${CMAKE_CURRENT_BINARY_DIR}" test.proto
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Generating protobuf souce and header")
