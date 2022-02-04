#pragma once
#include "./Core/IOCPServer.h"


class EchoServer : public IOCPServer
{
public:
	void OnConnect(const uint32_t clientIndex) override;
	void OnClose(const uint32_t clientIndex) override;
	void OnReceive(const uint32_t clientIndex, const uint32_t size, const char* pData) override;
};

