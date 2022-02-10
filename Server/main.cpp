/* My Code */
#include "./EchoServer.h"

#include <iostream>
#include <string>

const uint16_t SERVER_PORT = 11021;
const uint32_t MAX_CLIENT = 100;		//�� �����Ҽ� �ִ� Ŭ���̾�Ʈ ��

int main()
{
	EchoServer IOCPEcho;

	//���� �ʱ�ȭ.
	IOCPEcho.InitSocket();

	//���ϰ� ���� �ּҸ� �����ϰ� ���.
	IOCPEcho.BindandListen(SERVER_PORT);

	IOCPEcho.Run(MAX_CLIENT);

	std::cout << "Whenever type Q / q to quit" << "\n";
	while (true)
	{
		std::string input;
		std::cin >> input;

		if (input == "q" || input == "Q")
		{
			break;
		}
		else
		{
			std::cout << "Ignore :: Invalid command\n";
		}
	}

	IOCPEcho.End();
	std::cout << "OK :: Server finished.\n";

	return 0;
}