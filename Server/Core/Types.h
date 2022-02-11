#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdint>

constexpr uint32_t	MAX_SOCKBUF			= 1024; // 패킷 크기 
constexpr uint32_t	MAX_WORKERTHREAD	= 4;	// 쓰레드 풀에 넣을 쓰레드 수
constexpr int MAX_SENDBUF_SIZE = 2048;

enum class eIOOperation
{
	RECV = 0,
	SEND,
};

//WSAOVERLAPPED구조체를 확장 시켜서 필요한 정보를 더 넣었다.
struct OverlappedEx
{
	WSAOVERLAPPED	m_wsaOverlapped;		//Overlapped I/O구조체
	SOCKET			m_socketClient;			//클라이언트 소켓
	WSABUF			m_wsaBuf;				//Overlapped I/O작업 버퍼
	eIOOperation	m_eOperation;			//작업 동작 종류
};