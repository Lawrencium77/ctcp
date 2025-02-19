cmake_minimum_required(VERSION 3.10)
project(MyProject LANGUAGES C)

option(DEBUG "Build with debugging flags" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(DEBUG)
    message(STATUS "Enabling debug flags")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DDEBUG")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ip
    ${CMAKE_SOURCE_DIR}/src/udp
    ${CMAKE_SOURCE_DIR}/src/udp/client
    ${CMAKE_SOURCE_DIR}/src/udp/server
)

add_executable(ip_server src/ip/server.c src/ip/ip_socket.c)
add_executable(ip_client src/ip/client.c src/ip/ip_client_utils.c src/ip/ip_socket.c)
add_executable(udp_daemon src/udp/server/daemon.c src/udp/checksum.c src/ip/ip_socket.c)
add_executable(udp_server src/udp/server/server.c src/udp/checksum.c src/ip/ip_socket.c)
add_executable(udp_client src/udp/client/client.c src/udp/checksum.c src/udp/client/client_ports.c src/ip/ip_client_utils.c src/ip/ip_socket.c)

set_target_properties(ip_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/ip"
    OUTPUT_NAME "server"
)
set_target_properties(ip_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/ip"
    OUTPUT_NAME "client"
)
set_target_properties(udp_daemon PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/udp"
    OUTPUT_NAME "daemon"
)
set_target_properties(udp_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/udp"
    OUTPUT_NAME "server"
)
set_target_properties(udp_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/udp"
    OUTPUT_NAME "client"
)

set_target_properties(udp_daemon PROPERTIES FOLDER "udp")
set_target_properties(udp_server PROPERTIES FOLDER "udp")
set_target_properties(udp_client PROPERTIES FOLDER "udp")
