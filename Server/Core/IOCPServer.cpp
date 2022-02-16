#include "./IOCPServer.h"

#include <iostream>

IOCPServer::IOCPServer()
{}

IOCPServer::~IOCPServer()
{
	for (auto& client : m_pSessions)
	{
		if (nullptr != client)
		{
			if (client->IsConnected())
			{
				client->Close(true);
			}
			delete client;
			client = nullptr;
		}
	}

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

	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, MAX_WORKERTHREAD);
	if (!m_hIOCP)
	{
		std::cerr << "Error :: CreateIOCompletionPort() :: " << WSAGetLastError() << "\n";
		return false;
	}
	auto hResult = CreateIoCompletionPort((HANDLE)m_listenSocket, m_hIOCP, 0, 0);
	if (!hResult)
	{
		std::cerr << "Error :: listen socket IOCP bind falied :: " << WSAGetLastError() << "\n";
	}

	std::cout << "OK :: Sever Registration\n";

	return true;
}

bool IOCPServer::StartServer(const uint32_t maxClientCount)
{
	createClient(maxClientCount);

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

	isSucceed = createSenderThread();
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
	closesocket(m_listenSocket);
	if (m_accepterThread.joinable())
	{
		m_accepterThread.join();
	}

	m_isSenderRun = false;
	if (m_senderThread.joinable())
	{
		m_senderThread.join();
	}
}

void IOCPServer::createClient(const uint32_t maxClientCount)
{
	for (uint32_t i = 0; i < maxClientCount; ++i)
	{
		m_pSessions.emplace_back(new Session(i, m_hIOCP));
	}
}

bool IOCPServer::createWorkerThread()
{
	for (int i = 0; i < MAX_WORKERTHREAD; i++)
	{
		m_IOWorkerThreads.emplace_back([this]()->void { workerThread(); });
	}

	std::cout << "OK :: WorkerThread Running\n";

	return true;
}

bool IOCPServer::createAccepterThread()
{
	m_accepterThread = std::thread([this]()->void { accepterThread(); });
	std::cout << "OK :: AccepterThread Running\n";

	return true;
}

bool IOCPServer::createSenderThread()
{
	m_senderThread = std::thread([this]()->void { senderThread(); });
	std::cout << "OK :: SenderTread Running\n";

	return true;
}

Session* IOCPServer::getEmptySession()
{
	for (auto& client : m_pSessions)
	{
		if (!client->IsConnected())
		{
			return client;
		}
	}

	return nullptr;
}

Session* IOCPServer::getSession(uint32_t sessionIndex)
{
	return m_pSessions[sessionIndex];
}

bool IOCPServer::sendMsg(uint32_t sessionIndex, char* pMsg, uint32_t msgLen)
{
	auto pSession = getSession(sessionIndex);

	return pSession->SendMsg(pMsg, msgLen);
}

void IOCPServer::workerThread()
{
	DWORD byteTransferred		(0);
	Session* pSession			(nullptr);
	LPOVERLAPPED lpOverlapped	(nullptr);

	while (m_isWorkerRun)
	{
		bool isSucceed = GetQueuedCompletionStatus(m_hIOCP,
			&byteTransferred, 
			(PULONG_PTR)&pSession, 
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

		OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

		// Client가 접속 종료
		if (false == isSucceed || (pOverlappedEx->m_eOperation != eIOOperation::ACCEPT && 0 == byteTransferred))
		{
			closeSocket(pSession);
			continue;
		}

		switch (pOverlappedEx->m_eOperation)
		{
			// Overlapped Receive 작업 후처리
		case eIOOperation::ACCEPT:
			{
				// TODO: Implement this.
				pSession = getSession(pOverlappedEx->m_sessionIndex);
				if (pSession->OnAcceptComplete())
				{
					++m_clientCount;
					
					OnConnect(pSession->GetIndex());
				}
				else
				{
					closeSocket(pSession, true);
				}
			}
			break;
		case eIOOperation::RECV:
			{
				OnReceive(pSession->GetIndex(), byteTransferred, pSession->RecvBuffer());
				pSession->BindRecv();
			}
			break;
			// Overlapped Send 작업 후처리
		case eIOOperation::SEND:
			{
				pSession->OnSendComplete(byteTransferred);
			}
			break;
			// 유효하지 않은 작업코드가 저장되어 있는 경우
		default:
			{
				std::cerr << "Error :: Invalid Socket Operation :: Socket(" << (int)pSession->GetIndex() << ")\n";
			}
			break;
		}
	}
}

void IOCPServer::accepterThread()
{
	while (m_isAccepterRun)
	{
		auto curTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

		for (auto client : m_pSessions)
		{
			if (client->IsConnected())
			{
				continue;
			}

			if ((uint64_t)curTimeSec < client->GetLastClosedTimeSec())
			{
				continue;
			}

			auto diff = curTimeSec - client->GetLastClosedTimeSec();
			if (diff <= RE_USE_SESSION_WAIT_TIMESEC)
			{
				continue;
			}

			client->PostAccept(m_listenSocket, curTimeSec);
		}
	}
}

void IOCPServer::senderThread()
{
	while (m_isSenderRun)
	{
		for (auto& client : m_pSessions)
		{
			if (!client->IsConnected())
			{
				continue;
			}
			client->SendIO();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void IOCPServer::closeSocket(Session* pSession, bool isForce)
{
	auto clientIndex = pSession->GetIndex();
	pSession->Close(isForce);

	OnClose(clientIndex);
}