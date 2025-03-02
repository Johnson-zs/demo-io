cmake_minimum_required(VERSION 3.16)
project(unix-domain-demo VERSION 1.0)

add_executable(server
   unix_server_socket.c
)

add_executable(client
   unix_client_socket.c
)
