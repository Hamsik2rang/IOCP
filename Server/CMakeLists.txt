cmake_minimum_required(VERSION 3.22.2)

project(Server)

add_executable(Server 
	main.cpp 
	EchoServer.cpp

	./Core/IOCPServer.cpp
)
