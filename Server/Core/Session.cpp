#include "Session.h"

#include <iostream>

Session::Session()
{
	ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
	m_socket = INVALID_SOCKET;
}

Session::Session(uint32_t index)
	:Session()
{
	m_index = index;
}

Session::~Session()
{

}

void Session::InitIndex(uint32_t index)
{
	ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
	m_socket = INVALID_SOCKET;
	m_index = index;
}

uint32_t Session::GetIndex() const
{
	return m_index;
}

bool Session::IsConnected() const
{
	return m_socket != INVALID_SOCKET;
}

SOCKET Session::GetSocket() const
{
	return m_socket;
}

char* Session::RecvBuffer()
{
	return m_recvBuf;
}

bool Session::OnConnect(HANDLE hIOCP, SOCKET socket)
{
	m_socket = socket;

	if (!BindIOCompletionPort(hIOCP))
	{
		return false;
	}

	return BindRecv();
}

bool Session::BindIOCompletionPort(HANDLE hIOCP)
{
	auto hResult = CreateIoCompletionPort((HANDLE)m_socket, hIOCP, (ULONG_PTR)this, 0);
	if (hResult == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error :: BindIOCompletionPort :: " << WSAGetLastError() << "\n";
		return false;
	}

	return true;
}

bool Session::BindRecv()
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

bool Session::SendMsg(const char* pMsg, const uint32_t len)
{
	auto sendOverlappedEx = new OverlappedEx();
	ZeroMemory(sendOverlappedEx, sizeof(OverlappedEx));
	sendOverlappedEx->m_wsaBuf.len = len;
	sendOverlappedEx->m_wsaBuf.buf = new char[len];
	CopyMemory(sendOverlappedEx->m_wsaBuf.buf, pMsg, len);
	sendOverlappedEx->m_eOperation = eIOOperation::SEND;

	std::lock_guard<std::mutex> lock(m_mutex);

	m_pSendDataQueue.push(sendOverlappedEx);

	return true;
}

bool Session::SendIO()
{
	if (m_isSending || m_pSendDataQueue.empty())
	{
		return false;
	}

	m_isSending = true;

	std::lock_guard<std::mutex> lock(m_mutex);

	auto sendOverlappedEx = m_pSendDataQueue.front();
	m_pSendDataQueue.pop();
	DWORD recvBytes(0);

	int result = WSASend(m_socket,
		&sendOverlappedEx->m_wsaBuf,
		1,
		&recvBytes,
		0,
		&sendOverlappedEx->m_wsaOverlapped,
		nullptr);

	if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::cerr << "Error :: WSASend()\n";
		return false;
	}

	return true;
}

void Session::OnSendComplete(DWORD byteTransferred)
{
	std::cout << "Send :: " << byteTransferred << "Bytes" << "\n";
	m_isSending = false;
}

void Session::Close(bool isForce)
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