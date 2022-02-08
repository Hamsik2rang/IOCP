#pragma once

#include "Types.h"
#include <iostream>

class ClientInfo
{
private:
	SOCKET			m_socket;
	OverlappedEx	m_recvOverlappedEx;
	uint32_t		m_index = 0;
	char			m_recvBuf[MAX_SOCKBUF];

public:
	ClientInfo()
	{
		ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
		m_socket = INVALID_SOCKET;
	}
	
	ClientInfo(uint32_t index)
		:ClientInfo()
	{
		m_index = index;
	}

	void InitIndex(uint32_t index)
	{
		m_index = index;
	}

	uint32_t GetIndex() const
	{
		return m_index;
	}

	bool IsConnected() const
	{
		return m_socket != INVALID_SOCKET;
	}

	SOCKET GetSocket() const
	{
		return m_socket;
	}

	char* RecvBuffer()
	{
		return m_recvBuf;
	}

	bool OnConnect(HANDLE hIOCP, SOCKET socket)
	{
		m_socket = socket;

		if (!BindIOCompletionPort(hIOCP))
		{
			return false;
		}

		return BindRecv();
	}

	bool BindIOCompletionPort(HANDLE hIOCP)
	{
		auto hResult = CreateIoCompletionPort((HANDLE)m_socket, hIOCP, (ULONG_PTR)this, 0);
		if (hResult == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Error :: BindIOCompletionPort :: " << WSAGetLastError() << "\n";
			return false;
		}

		return true;
	}

	bool BindRecv()
	{
		m_recvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		m_recvOverlappedEx.m_wsaBuf.buf = m_recvBuf;
		m_recvOverlappedEx.m_eOperation = eIOOperation::RECV;

		DWORD flags		(0);
		DWORD recvBytes	(0);

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
	
	bool SendMsg(const char* pMsg, const uint32_t len)
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

	void SendCompeleted(DWORD byteTransferred)
	{
		std::cout << "Send :: " << byteTransferred << "Bytes" << "\n";
	}

	void Close(bool isForce)
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
};