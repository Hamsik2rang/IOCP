#include "ClientInfo.h"

#include <iostream>

ClientInfo::ClientInfo()
{
	ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
	m_socket = INVALID_SOCKET;
}

ClientInfo::ClientInfo(uint32_t index)
	:ClientInfo()
{
	m_index = index;
}

void ClientInfo::InitIndex(uint32_t index)
{
	m_index = index;
}

uint32_t ClientInfo::GetIndex() const
{
	return m_index;
}

bool ClientInfo::IsConnected() const
{
	return m_socket != INVALID_SOCKET;
}

SOCKET ClientInfo::GetSocket() const
{
	return m_socket;
}

char* ClientInfo::RecvBuffer()
{
	return m_recvBuf;
}

bool ClientInfo::OnConnect(HANDLE hIOCP, SOCKET socket)
{
	m_socket = socket;

	if (!BindIOCompletionPort(hIOCP))
	{
		return false;
	}

	return BindRecv();
}

bool ClientInfo::BindIOCompletionPort(HANDLE hIOCP)
{
	auto hResult = CreateIoCompletionPort((HANDLE)m_socket, hIOCP, (ULONG_PTR)this, 0);
	if (hResult == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error :: BindIOCompletionPort :: " << WSAGetLastError() << "\n";
		return false;
	}

	return true;
}

bool ClientInfo::BindRecv()
{
	m_recvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
	m_recvOverlappedEx.m_wsaBuf.buf = m_recvBuf;
	m_recvOverlappedEx.m_eOperation = eIOOperation::RECV;

	DWORD flags(0);
	DWORD recvBytes(0);

	int result = WSARecv(m_socket,
		&(m_recvOverlappedEx.m_wsaBuf),
		1,
		&recvBytes,
		&flags,
		&(m_recvOverlappedEx.m_wsaOverlapped),
		nullptr);


	if (SOCKET_ERROR == result && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::cerr << "Error :: WSARecv() :: " << WSAGetLastError() << "\n";
		return false;
	}

	return true;
}

bool ClientInfo::SendMsg(const char* pMsg, const uint32_t len)
{
	OverlappedEx* pSendOverlappedEx = new OverlappedEx;
	ZeroMemory(pSendOverlappedEx, sizeof(OverlappedEx));
	pSendOverlappedEx->m_wsaBuf.len = len;
	pSendOverlappedEx->m_wsaBuf.buf = new char[len];
	CopyMemory(pSendOverlappedEx->m_wsaBuf.buf, pMsg, len);
	pSendOverlappedEx->m_eOperation = eIOOperation::SEND;

	DWORD recvBytes(0);

	int result = WSASend(m_socket,
		&(pSendOverlappedEx->m_wsaBuf),
		1,
		&recvBytes,
		0,
		&(pSendOverlappedEx->m_wsaOverlapped),
		nullptr);

	if (SOCKET_ERROR == result && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "Error :: WSASend() :: " << WSAGetLastError() << "\n";
		return false;
	}

	return true;
}

void ClientInfo::SendIO()
{
	std::lock_guard<std::mutex> lock(m_mutex);



}

void ClientInfo::OnSendComplete(DWORD byteTransferred)
{
	std::cout << "Send :: " << byteTransferred << "Bytes" << "\n";
}


void ClientInfo::Close(bool isForce)
{
	linger socketLinger{ 0 };

	if (isForce)
	{
		socketLinger.l_onoff = 1;
	}

	shutdown(m_socket, SD_BOTH);
	setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char*)&socketLinger, sizeof(socketLinger));

	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}