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
	createClient(maxClientCount);

	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, MAX_WORKERTHREAD);
	if (!m_hIOCP)
	{
		std::cerr << "Error :: CreateIOCompletionPort() :: " << WSAGetLastError() << "\n";
		return false;
	}

	bool isSucceed = createWorkerThread();
	if (!isSucceed)
	{
		return false;
	}

	isSucceed = createAccepterThread();
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
	CloseHandle(m_hIOCP);

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

void IOCPServer::createClient(const uint32_t maxClientCount)
{
	for (uint32_t i = 0; i < maxClientCount; ++i)
	{
		m_clientInfos.emplace_back(i);
	}
}

bool IOCPServer::createWorkerThread()
{
	for (int i = 0; i < MAX_WORKERTHREAD; i++)
	{
		m_IOWorkerThreads.emplace_back([this]()->void { workerThread(); });
	}

	std::cout << "OK :: WokrerThread Running\n";

	return true;
}

bool IOCPServer::createAccepterThread()
{
	m_accepterThread = std::thread([this]()->void { accepterThread(); });
	std::cout << "OK :: AccepterThread Running\n";

	return true;
}

ClientInfo* IOCPServer::getEmptyClientInfo()
{
	for (auto& client : m_clientInfos)
	{
		if (!client.IsConnected())
		{
			return &client;
		}
	}

	return nullptr;
}

ClientInfo* IOCPServer::getClientInfo(uint32_t sessionIndex)
{
	return &m_clientInfos[sessionIndex];
}

bool IOCPServer::sendMsg(uint32_t sessionIndex, char* pMsg, uint32_t msgLen)
{
	auto pClientInfo = getClientInfo(sessionIndex);

	return pClientInfo->SendMsg(pMsg, msgLen);
}

void IOCPServer::workerThread()
{
	DWORD byteTransferred		(0);
	ClientInfo* pClientInfo		(nullptr);
	LPOVERLAPPED lpOverlapped	(nullptr);

	while (m_isWorkerRun)
	{
		bool isSucceed = GetQueuedCompletionStatus(m_hIOCP,
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
			closeSocket(pClientInfo);
			continue;
		}

		OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;
		switch (pOverlappedEx->m_eOperation)
		{
			// Overlapped Receive 작업 후처리
		case eIOOperation::RECV:
			{
				OnReceive(pClientInfo->GetIndex(), byteTransferred, pClientInfo->RecvBuffer());
				pClientInfo->BindRecv();
			}
			break;
			// Overlapped Send 작업 후처리
		case eIOOperation::SEND:
			{
				delete[] pOverlappedEx->m_wsaBuf.buf;
				delete pOverlappedEx;
				pClientInfo->SendCompeleted(byteTransferred);
			}
			break;
			// 유효하지 않은 작업코드가 저장되어 있는 경우
		default:
			{
				std::cerr << "Error :: Invalid Socket Operation :: Socket(" << (int)pClientInfo->GetIndex() << ")\n";
			}
			break;
		}
	}
}

void IOCPServer::accepterThread()
{
	SOCKADDR_IN clientAddr;
	socklen_t clientAddrLen = (socklen_t)sizeof(clientAddr);

	while (m_isAccepterRun)
	{
		ClientInfo* pClientInfo = getEmptyClientInfo();
		if (!pClientInfo)
		{
			std::cerr << "Error :: Client Full\n";
			break;
		}

		auto clientSocket = accept(m_listenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
		if (INVALID_SOCKET == clientSocket)
		{
			continue;
		}

		if (false == pClientInfo->OnConnect(m_hIOCP, clientSocket))
		{
			pClientInfo->Close(true);
			break;
		}
		OnConnect(pClientInfo->GetIndex());
		++m_clientCount;
	}
}

void IOCPServer::closeSocket(ClientInfo * pClientInfo, bool isForce)
{
	auto clientIndex = pClientInfo->GetIndex();
	pClientInfo->Close(isForce);

	OnClose(clientIndex);
}