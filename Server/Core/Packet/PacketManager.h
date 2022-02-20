#pragma once

#include <queue>
#include <mutex>
#include <utility>

#include "../Types.h"
#include "Packet.h"


class PacketManager
{
private:
	std::queue<Packet> m_packetQueue;
	std::mutex m_mutex;

	ePacketCode decode();
public:
	void Process();
	void EnqueuePacket(Packet packet);
};