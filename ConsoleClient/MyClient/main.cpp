#pragma comment(lib, "ws2_32.lib")
#include <cstdio>
#include <cstring>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#define BUF_SIZE 1024

void ErrorHandling(const char* errmsg)
{
	fputs(errmsg, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char** argv)
{
	WSADATA wsaData{};
	SOCKET sock{};
	SOCKADDR_IN sockAddr{};
	char buf[BUF_SIZE] = "";

	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ErrorHandling("WSAStartup() error");
	}

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		ErrorHandling("WSASocket() error");
	}

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(sockAddr.sin_family, argv[1], &sockAddr.sin_addr.s_addr);

	if (connect(sock, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() error");
	}

	while (true)
	{
		printf("Type (Q/q to Quit): ");
		scanf_s("%s", buf, BUF_SIZE);

		if (!strcmp(buf, "Q\n") || !strcmp(buf, "q\n"))
		{
			break;
		}

		int strLen = strlen(buf);
		send(sock, buf, strLen, 0);
		int readLen = 0;

		while (true)
		{
			readLen += recv(sock, buf, BUF_SIZE - 1, 0);
			if (readLen >= strLen)
			{
				break;
			}
		}

		buf[strLen = 0];
		printf("Message from server: %s", buf);
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}