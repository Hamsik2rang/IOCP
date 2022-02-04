/* My Code */
#include "./EchoServer.h"

#include <iostream>
#include <string>

const uint16_t SERVER_PORT = 11021;
const uint16_t MAX_CLIENT = 100;		//총 접속할수 있는 클라이언트 수

int main()
{
	EchoServer IOCPEcho;

	//소켓 초기화.
	IOCPEcho.InitSocket();

	//소켓과 서버 주소를 연결하고 등록.
	IOCPEcho.BindandListen(SERVER_PORT);

	IOCPEcho.StartServer(MAX_CLIENT);

	while (true)
	{
		printf("Type Q/q to quit: ");
		std::string input;
		std::cin >> input;

		if (input == "q" || input == "Q")
		{
			break;
		}
		else
		{
			std::cout << "Invalid command ignored.\n";
		}
	}

	IOCPEcho.DestroyThread();
	return 0;
}