#include "./IOCPServer.h"

#include <iostream>

IOCPServer::IOCPServer()
{}

IOCPServer::~IOCPServer()
{
	WSACleanup();
}

bool IOCPServer::InitSocket()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result)
	{
		std::cerr << "Error :: WSAStartup() :: " << WSAGetLastError() << "\n";
		return false;
	}

	m_listenSocket = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

	if (INVALID_SOCKET == m_listenSocket)
	{
		std::cerr << "Error :: WSASocket() :: " << WSAGetLastError() << "\n";
		return false;
	}

	std::cout << "OK :: Socket Initialization\n";

	return true;
}

bool IOCPServer::BindandListen(int bindPort)
{
	SOCKADDR_IN serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(bindPort);

	int result = bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (SOCKET_ERROR == result)
	{
		std::cerr << "Error :: bind() :: " << WSAGetLastError() << "\n";
		return false;
	}

	result = listen(m_listenSocket, 5);
	if (SOCKET_ERROR == result)
	{
		std::cerr << "Error :: listen() :: " << WSAGetLastError() << "\n";
		return false;
	}

	std::cout << "OK :: Sever Registration\n";

	return true;
}

bool IOCPServer::StartServer(const uint32_t maxClientCount)
{

	CreateClient(maxClientCount);

	m_IOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, MAX_WORKERTHREAD);
	if (!m_IOCPHandle)
	{
		std::cerr << "Error :: CreateIoCompletionPort() :: " << WSAGetLastError() << "\n";
		return false;
	}

	bool isSucceed = CreateWorkerThread();
	if (!isSucceed)
	{
		return false;
	}

	isSucceed = CreateAccepterThread();
	if (!isSucceed)
	{
		return false;
	}

	std::cout << "OK :: Server Run\n";

	return true;
}

void IOCPServer::DestroyThread()
{
	m_isWorkerRun = false;
	CloseHandle(m_IOCPHandle);

	for (auto& th : m_IOWorkerThreads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	m_isAccepterRun = false;
	if (m_accepterThread.joinable())
	{
		m_accepterThread.join();
	}
}

void IOCPServer::CreateClient(const uint32_t maxClientCount)
{
	for (uint32_t i = 0; i < maxClientCount; ++i)
	{
		m_clientInfos.emplace_back();
	}
}

bool IOCPServer::CreateWorkerThread()
{
	for (int i = 0; i < MAX_WORKERTHREAD; i++)
	{
		m_IOWorkerThreads.emplace_back([this]()->void { WorkerThread(); });
	}

	std::cout << "OK :: WokrerThread Running\n";

	return true;
}

bool IOCPServer::CreateAccepterThread()
{
	m_accepterThread = std::thread([this]()->void { AccepterThread(); });
	std::cout << "OK :: AccepterThread Running\n";

	return true;
}

ClientInfo* IOCPServer::GetEmptyClientInfo()
{
	for (auto& client : m_clientInfos)
	{
		if (INVALID_SOCKET == client.m_socketClient)
		{
			return &client;
		}
	}

	return nullptr;
}

bool IOCPServer::BindIOCompletionPort(ClientInfo* pClientInfo)
{
	auto resultHandle = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient, m_IOCPHandle, (ULONG_PTR)pClientInfo, 0);

	if (nullptr == resultHandle || resultHandle != m_IOCPHandle)
	{
		std::cerr << "Error :: BindIOCompletionPort() :: " << WSAGetLastError() << "\n";
		return false;
	}

	return true;
}

bool IOCPServer::BindRecv(ClientInfo* pClientInfo)
{
	DWORD flag		(0);
	DWORD recvBytes	(0);

	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_recvBuf;
	pClientInfo->m_stRecvOverlappedEx.m_eOperation = eIOOperation::RECV;

	int result = WSARecv(pClientInfo->m_socketClient, 
		&(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
		1, 
		&recvBytes, 
		&flag,  
		&(pClientInfo->m_stRecvOverlappedEx.m_wsaOverlapped),
		nullptr);

	if (SOCKET_ERROR == result && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::cerr << "Error :: WSARecv() :: " << WSAGetLastError() << "\n";
		return false;
	}

	return false;
}

bool IOCPServer::SendMsg(ClientInfo* pClientInfo, char* pMsg, int msgLen)
{
	DWORD recvBytes(0);

	CopyMemory(pClientInfo->m_sendBuf, pMsg, msgLen);

	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = msgLen;
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->m_sendBuf;
	pClientInfo->m_stSendOverlappedEx.m_eOperation = eIOOperation::SEND;

	int result = WSASend(pClientInfo->m_socketClient,
		&(pClientInfo->m_stSendOverlappedEx.m_wsaBuf),
		1,
		&recvBytes,
		0,
		&(pClientInfo->m_stSendOverlappedEx.m_wsaOverlapped),
		nullptr);

	if (SOCKET_ERROR == result && WSAGetLastError() == ERROR_IO_PENDING)
	{
		std::cerr << "Error :: WSASend() :: " << WSAGetLastError() << "\n";
		return false;
	}

	return false;
}

void IOCPServer::WorkerThread()
{
	DWORD byteTransferred		(0);
	ClientInfo* pClientInfo		(nullptr);
	LPOVERLAPPED lpOverlapped	(nullptr);

	while (m_isWorkerRun)
	{
		bool isSucceed = GetQueuedCompletionStatus(m_IOCPHandle,
			&byteTransferred, 
			(PULONG_PTR)&pClientInfo, 
			&lpOverlapped, 
			INFINITE);

		// 쓰레드 종료
		if (true == isSucceed && nullptr == lpOverlapped && 0 == byteTransferred)
		{
			m_isWorkerRun = true;
			continue;
		}

		if (nullptr == lpOverlapped)
		{
			continue;
		}

		// Client가 접속 종료
		if (false == isSucceed || (true == isSucceed && 0 == byteTransferred))
		{
			std::cout << "Client disconnected...(Socket:" << (int)pClientInfo->m_socketClient << ")\n";
			CloseSocket(pClientInfo);
			continue;
		}

		OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;
		switch (pOverlappedEx->m_eOperation)
		{
			// Overlapped Receive 작업 후처리
		case eIOOperation::RECV:
			{
				pClientInfo->m_recvBuf[byteTransferred] = '\0';
#ifdef _UNUSED_PACKET_HEADER
				char msgWithoutHeader[MAX_SOCKBUF]{ 0 };
				CopyMemory(msgWithoutHeader, pClientInfo->m_recvBuf + 5, byteTransferred - 5);
				std::cout << "Recv :: (" << byteTransferred << ")Bytes :: " << msgWithoutHeader << "\n";
#elif
				std::cout << "Recv :: (" << byteTransferred << ")Bytes :: " << pClientInfo->m_recvBuf << "\n";
#endif
				//클라이언트에 메세지를 에코한다.
				SendMsg(pClientInfo, pClientInfo->m_recvBuf, byteTransferred);
				BindRecv(pClientInfo);
			}
			break;
			// Overlapped Send 작업 후처리
		case eIOOperation::SEND:
			{
#ifdef _UNUSED_PACKET_HEADER
				char msgWithoutHeader[MAX_SOCKBUF]{ 0 };
				CopyMemory(msgWithoutHeader, pClientInfo->m_sendBuf + 5, byteTransferred - 5);
				std::cout << "Send :: (" << byteTransferred << ")Bytes :: " << msgWithoutHeader << "\n";
#elif
				std::cout << "Send :: (" << byteTransferred << ")Bytes :: " << pClientInfo->m_sendBuf << "\n";
#endif
			}
			break;
			// 유효하지 않은 작업코드가 저장되어 있는 경우
		default:
			{
				std::cerr << "Error :: Invalid Socket Operation :: Socket(" << (int)pClientInfo->m_socketClient << ")\n";
			}
			break;
		}
	}
}

void IOCPServer::AccepterThread()
{
	SOCKADDR_IN clientAddr;
	socklen_t clientAddrLen = (socklen_t)sizeof(clientAddr);

	while (m_isAccepterRun)
	{
		ClientInfo* pClientInfo = GetEmptyClientInfo();
		if (!pClientInfo)
		{
			std::cerr << "Error :: Client Full\n";
			break;
		}

		pClientInfo->m_socketClient = accept(m_listenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
		if (INVALID_SOCKET == pClientInfo->m_socketClient)
		{
			continue;
		}

		bool isBound = BindIOCompletionPort(pClientInfo);
		if (!isBound)
		{
			break;
		}
		
		isBound = BindRecv(pClientInfo);
		if (!isBound)
		{
			break;
		}

		char clientIP[32]{ 0 };
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, sizeof(clientIP) - 1);
		std::cout << "Cilent connected :: IP(" << clientIP << ") :: Socket(" << pClientInfo->m_socketClient << ")\n";

		++m_clientCnt;
	}
}

void IOCPServer::CloseSocket(ClientInfo * pClientInfo, bool isForce)
{
	linger stLinger{ 0,0 };

	if (isForce)
	{
		stLinger.l_onoff = 1;
	}
	// Socket shutdown both send and recv.
	shutdown(pClientInfo->m_socketClient, SD_BOTH);
	// Set Socket linger option
	setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(linger));
	// Close Socket
	closesocket(pClientInfo->m_socketClient);
	pClientInfo->m_socketClient = INVALID_SOCKET;
}