#pragma comment(lib,"mswsock.lib")
#define NOMINMAX

#include "Session.h"

#include <MSWSock.h>
#include <iostream>
#include <numeric>

Session::Session()
{
	ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
	m_socket = INVALID_SOCKET;
	m_latestClosedTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

Session::Session(uint32_t index, HANDLE hIOCP)
	:Session()
{
	m_index = index;
	m_hIOCP = hIOCP;
	m_latestClosedTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

Session::~Session()
{}

void Session::InitIndex(uint32_t index, HANDLE hIOCP)
{
	ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
	m_socket = INVALID_SOCKET;
	m_index = index;
	m_hIOCP = hIOCP;
	m_latestClosedTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

uint64_t Session::GetLastClosedTimeSec() const
{
	return m_latestClosedTimeSec;
}

uint32_t Session::GetIndex() const
{
	return m_index;
}

bool Session::PostAccept(SOCKET listenSocket, const uint64_t curTimeSec)
{
	std::cout << "PostAccept :: Client Index : " << GetIndex() << "\n";
	
	m_latestClosedTimeSec = std::numeric_limits<uint64_t>::max();

	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_socket)
	{
		std::cerr << "Error :: Client Socket WSASocket() Error : " << WSAGetLastError() << "\n";
		return false;
	}

	ZeroMemory(&m_acceptContext, sizeof(OverlappedEx));

	DWORD bytes(0);
	DWORD flags(0);
	m_acceptContext.m_wsaBuf.buf = nullptr;
	m_acceptContext.m_wsaBuf.len = 0;
	m_acceptContext.m_eOperation = eIOOperation::ACCEPT;
	m_acceptContext.m_sessionIndex = m_index;

	//AcceptEx
	if (false == AcceptEx(listenSocket,
		m_socket,
		m_acceptBuf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&bytes,
		&m_acceptContext.m_wsaOverlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cerr << "Error :: AcceptEx() :: " << WSAGetLastError() << "\n";
			return false;
		}
	}

	return true;
}

bool Session::OnAcceptComplete()
{
	// Print
	std::cout << "OnAcceptComplete :: SessionIndex(" << m_index << ")\n";

	if (!OnConnect(m_hIOCP, m_socket))
	{
		return false;
	}

	SOCKADDR_IN clientAddr{};
	int addrLen = sizeof(SOCKADDR_IN);
	char clientIP[32]{0};

	inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, clientIP, sizeof(clientIP) - 1);
	std::cout << "Connect :: IP : " << clientIP << " :: SOCKET : " << (int)m_socket;
	
	return true;
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
	auto pSendOverlappedEx = new OverlappedEx;
	ZeroMemory(pSendOverlappedEx, sizeof(OverlappedEx));
	pSendOverlappedEx->m_wsaBuf.len = len;
	pSendOverlappedEx->m_wsaBuf.buf = new char[len];
	CopyMemory(pSendOverlappedEx->m_wsaBuf.buf, pMsg, len);
	pSendOverlappedEx->m_eOperation = eIOOperation::SEND;

	std::lock_guard<std::mutex> lock(m_mutex);

	m_pSendDataQueue.push(pSendOverlappedEx);

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

	auto pSendOverlappedEx = m_pSendDataQueue.front();
	DWORD recvBytes(0);


	int result = WSASend(m_socket,
		&(pSendOverlappedEx->m_wsaBuf),
		1,
		&recvBytes,
		0,
		&(pSendOverlappedEx->m_wsaOverlapped),
		nullptr);

	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "Error :: WSASend()" << WSAGetLastError() << "\n";
		return false;
	}

	return true;
}

void Session::OnSendComplete(DWORD byteTransferred)
{
	std::cout << "Send :: " << byteTransferred << "Bytes" << "\n";

	std::lock_guard<std::mutex> lock(m_mutex);

	auto pSendCompletedOverlappedEx = m_pSendDataQueue.front();
	m_pSendDataQueue.pop();

	delete[] pSendCompletedOverlappedEx->m_wsaBuf.buf;
	delete pSendCompletedOverlappedEx;

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

	m_latestClosedTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}