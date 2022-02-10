#pragma once

#include <queue>
#include <mutex>

#include "./Core/IOCPServer.h"
#include "./Core/Packet.hpp"


class EchoServer : public IOCPServer
{
private:
	std::queue<Packet>	m_packetQueue;
	std::thread			m_processThread;
	std::mutex			m_mutex;
	bool				m_isProcessRun;

	void processPacket();
	Packet dequeuePacket();

public:
	void OnConnect(const uint32_t clientIndex) override;
	void OnClose(const uint32_t clientIndex) override;
	void OnReceive(const uint32_t clientIndex, const uint32_t size, const char* pData) override;

	void Run(uint32_t maxClient);
	void End();
};

