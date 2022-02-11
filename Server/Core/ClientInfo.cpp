#include "ClientInfo.h"

#include <iostream>

ClientInfo::ClientInfo()
{
	ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
	ZeroMemory(&m_sendOverlappedEx, sizeof(OverlappedEx));
	m_socket = INVALID_SOCKET;
	m_bufPos = 0;
}

ClientInfo::ClientInfo(uint32_t index)
	:ClientInfo()
{
	m_index = index;
}

ClientInfo::~ClientInfo()
{

}

void ClientInfo::InitIndex(uint32_t index)
{
	ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
	m_socket = INVALID_SOCKET;
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
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_bufPos + len > MAX_SENDBUF_SIZE)
	{
		m_bufPos = 0;	// Why?
	}
	CopyMemory(m_dataBuf, pMsg, len);
	
	m_bufPos += len;

	return true;
}

bool ClientInfo::SendIO()
{
	if (0 >= m_bufPos || m_isSending)
	{
		return false;
	}

	m_isSending = true;

	std::lock_guard<std::mutex> lock(m_mutex);

	CopyMemory(m_sendingBuf, m_dataBuf, m_bufPos);
	m_sendOverlappedEx.m_wsaBuf.buf = m_sendingBuf;
	m_sendOverlappedEx.m_wsaBuf.len = m_bufPos;
	m_sendOverlappedEx.m_eOperation = eIOOperation::SEND;

	DWORD recvBytes(0);

	int result = WSASend(m_socket,
		&m_sendOverlappedEx.m_wsaBuf,
		1,
		&recvBytes,
		0,
		&m_sendOverlappedEx.m_wsaOverlapped,
		nullptr);

	if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::cerr << "Error :: WSASend()\n";
		return false;
	}

	return true;
}

void ClientInfo::OnSendComplete(DWORD byteTransferred)
{
	std::cout << "Send :: " << byteTransferred << "Bytes" << "\n";
	m_isSending = false;
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