#pragma once

#include <mutex>
#include <queue>
#include <utility>

#include "Types.h"

class Session
{
private:
	HANDLE			m_hIOCP;
	SOCKET			m_socket;
	OverlappedEx	m_acceptContext;
	OverlappedEx	m_recvOverlappedEx;
	uint32_t		m_index = 0;
	uint64_t		m_latestClosedTimeSec = 0;
	char			m_recvBuf[MAX_SOCKBUF];
	char			m_acceptBuf[64];
	
	std::mutex		m_mutex;
	bool			m_isSending = false;
	
	std::queue<OverlappedEx*> m_pSendDataQueue;

public:
	Session();
	Session(uint32_t index, HANDLE hIOCP);
	~Session();
	void InitIndex(uint32_t index, HANDLE hIOCP);

	uint64_t GetLastClosedTimeSec() const;
	uint32_t GetIndex() const;
	SOCKET GetSocket() const;
	char* RecvBuffer();

	bool PostAccept(SOCKET listenSocket, const uint64_t curTimeSec);
	bool OnAcceptComplete();
	bool IsConnected() const;
	bool OnConnect(HANDLE hIOCP, SOCKET socket);
	bool BindIOCompletionPort(HANDLE hIOCP);
	bool BindRecv();
	bool SendMsg(const char* pMsg, const uint32_t len);
	bool SendIO();

	void OnSendComplete(DWORD byteTransferred);
	void Close(bool isForce);
};