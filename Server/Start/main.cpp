/* My Code */
#include "./EchoServer.h"

#include <iostream>
#include <string>

const uint16_t SERVER_PORT = 11021;
const uint16_t MAX_CLIENT = 100;		//�� �����Ҽ� �ִ� Ŭ���̾�Ʈ ��

int main()
{
	EchoServer IOCPEcho;

	//���� �ʱ�ȭ.
	IOCPEcho.InitSocket();

	//���ϰ� ���� �ּҸ� �����ϰ� ���.
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