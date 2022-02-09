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

	Packet packet;
	packet.Set(clientIndex, size, pData);

	std::lock_guard<std::mutex> guard(m_mutex);
	m_packetQueue.push(packet);
}

void EchoServer::Run(uint32_t maxClient)
{
	m_isProcessRun = true;
	
	m_processThread = std::thread([this]()->void {processPacket(); });

	StartServer(maxClient);
}

void EchoServer::End()
{
	m_isProcessRun = false;

	if (m_processThread.joinable())
	{
		m_processThread.join();
	}
	DestroyThread();
}

void EchoServer::processPacket()
{
	while (m_isProcessRun)
	{
		auto packet = dequeuePacket();
		if (0 != packet.m_dataSize)
		{
			sendMsg(packet.m_sessionIndex, packet.m_pData, packet.m_dataSize);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

Packet EchoServer::dequeuePacket()
{
	Packet packet;
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_packetQueue.empty())
	{
		return packet;
	}

	auto frontPacket = m_packetQueue.front();
	packet.Set(frontPacket);

	frontPacket.Release();
	m_packetQueue.pop();

	return packet;
}