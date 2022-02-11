#pragma once
#pragma comment(lib, "ws2_32")

#include <thread>
#include <vector>

#include "Types.h"
#include "ClientInfo.h"

#define _UNUSED_PACKET_HEADER

class IOCPServer
{
private:
	std::vector<ClientInfo*>	m_pClientInfos;
	std::vector<std::thread>	m_IOWorkerThreads;

	SOCKET			m_listenSocket;
	std::thread		m_accepterThread;
	std::thread		m_senderThread;
	int				m_clientCount = 0;
	HANDLE			m_hIOCP = INVALID_HANDLE_VALUE;
	bool			m_isWorkerRun = true;
	bool			m_isAccepterRun = true;
	bool			m_isSenderRun = true;
	char			m_socketBuf[MAX_SOCKBUF] = { 0 };

	void createClient(const uint32_t maxClientCount);
	bool createWorkerThread();
	bool createAccepterThread();
	bool createSenderThread();
	ClientInfo* getEmptyClientInfo();
	ClientInfo* getClientInfo(uint32_t sessionIndex);

	void workerThread();
	void accepterThread();
	void senderThread();
	void closeSocket(ClientInfo* pClientInfo, bool isForce = false);

protected:
	bool sendMsg(uint32_t sessionIndex, char* pMsg, uint32_t msgLen);

public:
	IOCPServer();
	~IOCPServer();

	bool InitSocket();
	bool BindandListen(int bindPort);
	bool StartServer(const uint32_t maxClientCount);
	void DestroyThread();

	virtual void OnConnect(const uint32_t clientIndex) = 0;
	virtual void OnClose(const uint32_t clientIndex) = 0;
	virtual void OnReceive(const uint32_t clientIndex, const uint32_t size, const char* pData) = 0;
};

