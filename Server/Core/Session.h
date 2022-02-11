#pragma once

#include <mutex>
#include <queue>
#include <utility>

#include "Types.h"

class Session
{
private:
	SOCKET			m_socket;
	OverlappedEx	m_recvOverlappedEx;
	uint32_t		m_index = 0;
	char			m_recvBuf[MAX_SOCKBUF];
	
	std::mutex		m_mutex;
	bool			m_isSending = false;
	
	std::queue<OverlappedEx*> m_pSendDataQueue;

public:
	Session();
	Session(uint32_t index);
	~Session();
	void InitIndex(uint32_t index);
	

	uint32_t GetIndex() const;
	SOCKET GetSocket() const;
	char* RecvBuffer();

	bool IsConnected() const;
	bool OnConnect(HANDLE hIOCP, SOCKET socket);
	bool BindIOCompletionPort(HANDLE hIOCP);
	bool BindRecv();
	bool SendMsg(const char* pMsg, const uint32_t len);
	bool SendIO();

	void OnSendComplete(DWORD byteTransferred);
	void Close(bool isForce);
};