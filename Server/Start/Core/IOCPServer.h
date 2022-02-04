#pragma once
#pragma comment(lib, "ws2_32")

#include <thread>
#include <vector>

#include "../Types.h"

#define _UNUSED_PACKET_HEADER

class IOCPServer
{
private:
	std::vector<ClientInfo>		m_clientInfos;
	std::vector<std::thread>	m_IOWorkerThreads;

	SOCKET			m_listenSocket;
	std::thread		m_accepterThread;
	int				m_clientCnt = 0;
	HANDLE			m_IOCPHandle = INVALID_HANDLE_VALUE;
	bool			m_isWorkerRun = true;
	bool			m_isAccepterRun = true;
	char			m_socketBuf[MAX_SOCKBUF] = { 0 };

	void CreateClient(const uint32_t maxClientCount);
	bool CreateWorkerThread();
	bool CreateAccepterThread();
	ClientInfo* GetEmptyClientInfo();
	bool BindIOCompletionPort(ClientInfo* pClientInfo);
	bool BindRecv(ClientInfo* pClientInfo);
	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int msgLen);

	void WorkerThread();
	void AccepterThread();
	void CloseSocket(ClientInfo* pClientInfo, bool isForce = false);

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

