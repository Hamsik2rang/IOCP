#pragma once

#include <mutex>

#include "Types.h"

class ClientInfo
{
private:


	SOCKET				m_socket;
	OverlappedEx		m_recvOverlappedEx;
	OverlappedEx		m_sendOverlappedEx;
	uint32_t			m_index = 0;
	char				m_recvBuf[MAX_SOCKBUF];

	// TODO: Check constructor error.
	std::mutex			m_mutex;
	char				m_dataBuf[MAX_SENDBUF_SIZE];
	char				m_sendingBuf[MAX_SENDBUF_SIZE];
	uint32_t			m_bufPos;
	bool				m_isSending = false;

public:
	ClientInfo();
	ClientInfo(uint32_t index);
	~ClientInfo();
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