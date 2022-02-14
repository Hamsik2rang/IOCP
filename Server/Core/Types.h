#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdint>

constexpr uint32_t	MAX_SOCKBUF			= 1024; // Size of packet
constexpr uint32_t	MAX_WORKERTHREAD	= 4;	// Size of Thread pool
constexpr uint64_t	RE_USE_SESSION_WAIT_TIMESEC = 3;	// Time interval to reuse session

enum class eIOOperation
{
	RECV = 0,
	SEND,
	ACCEPT,
};

// Extension of WSAOVERLAPPED structure to fill more informations needed.
struct OverlappedEx
{
	WSAOVERLAPPED	m_wsaOverlapped;		// Overlapped I/O structure
	SOCKET			m_socketClient;			// Client socket
	WSABUF			m_wsaBuf;				// Overlapped I/O buffer
	eIOOperation	m_eOperation;			// User-defined operation
	uint32_t		m_sessionIndex;			// Session Index
};