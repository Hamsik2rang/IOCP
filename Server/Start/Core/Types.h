#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdint>

constexpr uint32_t	MAX_SOCKBUF			= 1024; // ��Ŷ ũ�� 
constexpr uint32_t	MAX_WORKERTHREAD	= 4;	// ������ Ǯ�� ���� ������ ��

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

////Ŭ���̾�Ʈ ������ ������� ����ü
//struct ClientInfo
//{
//	SOCKET			m_socketClient			= INVALID_SOCKET;	//Cliet�� ����Ǵ� ����
//	OverlappedEx	m_recvOverlappedEx		= { 0 };			//RECV Overlapped I/O�۾��� ���� ����
//	OverlappedEx	m_sendOverlappedEx		= { 0 };			//SEND Overlapped I/O�۾��� ���� ����
//	char			m_recvBuf[MAX_SOCKBUF]	= { 0 };
//	char			m_sendBuf[MAX_SOCKBUF]	= { 0 };
//};
