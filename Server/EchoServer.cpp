#include "EchoServer.h"

#include <iostream>

void EchoServer::OnConnect(uint32_t clientIndex)
{
	std::cout << "Connect :: Client : " << clientIndex << "(Index)\n";
}

void EchoServer::OnClose(uint32_t clientIndex)
{
	std::cout << "Close :: Client : " << clientIndex << "(Index)\n";
}

void EchoServer::OnReceive(const uint32_t clientIndex, const uint32_t size, const char* pData)
{
	std::cout << "Recv :: Client : " << clientIndex << "(Index) :: " << pData << " (" << size << ")Bytes\n";
}

void EchoServer::Run(uint32_t MAX_CLIENT)
{
	m_isProcessRun = true;


	StartServer(MAX_CLIENT);
}
