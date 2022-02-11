#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdint>

constexpr uint32_t	MAX_SOCKBUF			= 1024; // ��Ŷ ũ�� 
constexpr uint32_t	MAX_WORKERTHREAD	= 4;	// ������ Ǯ�� ���� ������ ��
constexpr int MAX_SENDBUF_SIZE = 2048;

enum class eIOOperation
{
	RECV = 0,
	SEND,
};

//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
struct OverlappedEx
{
	WSAOVERLAPPED	m_wsaOverlapped;		//Overlapped I/O����ü
	SOCKET			m_socketClient;			//Ŭ���̾�Ʈ ����
	WSABUF			m_wsaBuf;				//Overlapped I/O�۾� ����
	eIOOperation	m_eOperation;			//�۾� ���� ����
};