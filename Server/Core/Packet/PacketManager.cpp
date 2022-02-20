#include "PacketManager.h"

ePacketCode PacketManager::decode()
{
	//TODO: Implement
	return ePacketCode();
}

void PacketManager::Process()
{
	m_mutex.lock();
	auto packet = m_packetQueue.front();
	m_packetQueue.pop();
	m_mutex.unlock();

}

void PacketManager::EnqueuePacket(Packet packet)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	m_packetQueue.push(packet);
}
