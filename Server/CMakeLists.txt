cmake_minimum_required(VERSION 3.21)

project(Server)

add_executable(Server 
	main.cpp 
	EchoServer.cpp

	"Core/Network/IOCPServer.cpp" 
	"Core/Session/Session.cpp" 
	"Core/Packet/Packet.h" 
	"Core/Packet/PacketManager.cpp")