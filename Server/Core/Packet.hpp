#pragma once

#include <cstdint>

class Packet
{
public:
	uint32_t	m_sessionIndex;
	uint32_t	m_dataSize;
	char*		m_pData;

	Packet()
		: m_sessionIndex(0), m_dataSize(0), m_pData(nullptr)
	{}

	Packet(Packet& p)
		:m_sessionIndex(p.m_sessionIndex), m_dataSize(p.m_dataSize), m_pData(p.m_pData)
	{}

	Packet(uint32_t sessionIndex, uint32_t dataSize, char* pData)
		:m_sessionIndex(sessionIndex), m_dataSize(dataSize), m_pData(pData)
	{}

	~Packet()
	{
		if (nullptr != m_pData)
		{
			delete m_pData;
			m_pData = nullptr;
		}
	}

	void Set(const Packet& p)
	{
		m_sessionIndex = p.m_sessionIndex;
		m_dataSize = p.m_dataSize;
		m_pData = p.m_pData;
	}

	void Set(uint32_t sessionIndex, uint32_t dataSize, char* pData)
	{
		m_sessionIndex = sessionIndex;
		m_dataSize = dataSize;
		m_pData = pData;
	}

	void Release()
	{
		if (m_pData)
		{
			delete m_pData;
			m_pData = nullptr;
		}
	}
};