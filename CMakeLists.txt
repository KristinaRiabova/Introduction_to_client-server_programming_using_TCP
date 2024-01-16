cmake_minimum_required(VERSION 3.26)
project(client_server_programming)

set(CMAKE_CXX_STANDARD 17)

add_executable(client_server_programming server.cpp
        client.cpp
        protocol.h)
