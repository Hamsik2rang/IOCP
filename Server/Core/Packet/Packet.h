#pragma once

///////////////////////////////////////////
/// Packet Codes
///////////////////////////////////////////
enum class ePacketCode : uint16_t
{
	ECHO = 1,

	// Log-in
	LOGIN_REQUEST = 201,
	LOGIN_RESPONSE = 202,

	// Enter room
	ROOM_ENTER_REQUEST = 206,
	ROOM_ENTER_RESPONSE = 207,
	ROOM_NEW_USER_NOTIFY = 208,
	ROOM_USER_LIST_NOTIFY = 209,

	// Leave room
	ROOM_LEAVE_REQUEST = 215,
	ROOM_LEAVE_RESPONSE = 216,
	ROOM_LEAVE_USER_NOTIFY = 217,

	// Chat in room
	ROOM_CHAT_REQUEST = 221,
	ROOM_CHAT_RESPONSE = 222,
	ROOM_CHAT_NOTIFY = 223,
};

enum class eErrorCode : uint16_t
{
	ERROR_NONE = 0,
	ERROR_CODE_USER_MGR_INVALID_USER_UNIQUEID = 112,
	ERROR_CODE_PACKET_CHANNER_IN_USER = 114,
	ERROR_CODE_PUBLIC_CHANNEL_INVALID_NUMBER = 115,
};

///////////////////////////////////////////
/// Packet Classes
///////////////////////////////////////////
class Packet
{
public:
	uint32_t	m_sessionIndex;
	uint32_t	m_dataSize;
	char* m_pData;

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

#pragma pack(push, 1)
struct PacketHeader
{
	uint32_t	m_length;
	uint32_t	m_code;
	uint8_t		m_type;
};

constexpr uint32_t MAX_PACKET_HEADER_LEN	= sizeof(PacketHeader);
constexpr uint32_t MAX_USER_ID_LEN			= 32;	// Max ID length
constexpr uint32_t MAX_USER_PW_LEN			= 32;	// Max Password length
constexpr uint32_t	MAX_CHAT_MESSAGE_SIZE	= 256;	// Max chatting message length

struct LoginRequest : public PacketHeader
{
	char m_id[MAX_USER_ID_LEN] = "";
	char m_pw[MAX_USER_PW_LEN] = "";
};

struct LoginResponse : public PacketHeader
{
	eErrorCode result;
};

struct RoomEnter : public PacketHeader
{
	uint32_t m_roomNum;
};

struct RoomLeaveRequest : public PacketHeader
{};

struct RoomLeaveResponse : public PacketHeader
{
	eErrorCode result;
};

struct PacketChat : public PacketHeader
{
	char m_pMessage[MAX_CHAT_MESSAGE_SIZE] = "";
};
#pragma pack(pop)