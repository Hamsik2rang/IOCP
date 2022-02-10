#pragma once

#include <mutex>

#include "Types.h"

class ClientInfo
{
private:
	SOCKET				m_socket;
	OverlappedEx		m_recvOverlappedEx;
	uint32_t			m_index = 0;
	char				m_recvBuf[MAX_SOCKBUF];
	static std::mutex	m_mutex;

public:
	ClientInfo();
	ClientInfo(uint32_t index);

	void InitIndex(uint32_t index);
	

	uint32_t GetIndex() const;
	SOCKET GetSocket() const;
	char* RecvBuffer();

	bool IsConnected() const;
	bool OnConnect(HANDLE hIOCP, SOCKET socket);
	bool BindIOCompletionPort(HANDLE hIOCP);
	bool BindRecv();
	bool SendMsg(const char* pMsg, const uint32_t len);
	void SendIO();

	void OnSendComplete(DWORD byteTransferred);
	void Close(bool isForce);
};