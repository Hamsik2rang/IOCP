#pragma once

#include <cstdint>


///////////////////////////////////////////
/// Packet Codes
///////////////////////////////////////////
enum class ePacketCode
{
	ECHO_FOR_DEV = 1,

	// Log-in
	LOGIN_REQ = 201,
	LOGIN_RES = 202,

	// Enter room
	ROOM_ENTER_REQ		= 206,
	ROOM_ENTER_RES		= 207,
	ROOM_NEW_USER_NTF	= 208,
	ROOM_USER_LIST_NTF	= 209,

	// Leave room
	ROOM_LEAVE_REQ		= 215,
	ROOM_LEAVE_RES		= 216,
	ROOM_LEAVE_USER_NTF = 217,

	// Chat in room
	ROOM_CHAT_REQ		= 221,
	ROOM_CHAT_RES		= 222,
	ROOM_CHAT_NOTIFY	= 223,
};

enum class eErrorCode
{
	ERROR_NONE = 0,
	ERROR_CODE_USER_MGR_INVALID_USER_UNIQUEID	= 112,
	ERROR_CODE_PACKET_CHANNER_IN_USER			= 114,
	ERROR_CODE_PUBLIC_CHANNEL_INVALID_NUMBER	= 115,
};

///////////////////////////////////////////
/// Packet Classes
///////////////////////////////////////////
class Packet
{
public:
	uint32_t	m_sessionIndex;
	uint32_t	m_dataSize;
	char*		m_pData;

	Packet()
		: m_sessionIndex(0), m_dataSize(0), m_pData(nullptr)
	{}

	void Set(const Packet& p)
	{
		m_sessionIndex = p.m_sessionIndex;
		m_dataSize = p.m_dataSize;
		m_pData = new char[m_dataSize];
		CopyMemory(m_pData, p.m_pData, m_dataSize);
	}

	void Set(uint32_t sessionIndex, uint32_t dataSize, const char* pData)
	{
		m_sessionIndex = sessionIndex;
		m_dataSize = dataSize;
		m_pData = new char[m_dataSize];
		CopyMemory(m_pData, pData, m_dataSize);
	}

	void Release()
	{
		if (nullptr != m_pData)
		{
			delete[] m_pData;
			m_pData = nullptr;
		}
	}
};