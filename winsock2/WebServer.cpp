#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define BUFFER_SIZE 1024
#define HOST "127.0.0.1"
#define PORT_NUM 80
#define HTML "\
<html> \
<head>\
<title>WebServer</title>\
<head>\
<body>\
<h1>Congratulations !</h1>\
<h1>You Got It !</h1>\
</body>\
</html>\
"

int main(int argc, char** argv)
{
	// define and init an server sockaddr
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT_NUM);

	// init socket dll

	WSADATA wsaData;
	WORD socketVersion = MAKEWORD(2, 2);

	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		printf("Init socket error!");
		exit(1);
	}

	// create socket

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (SOCKET_ERROR == ServerSocket)
	{
		printf("Create socket error!");
		exit(1);
	}

	// bind server socket host
	if (SOCKET_ERROR == bind(ServerSocket, (LPSOCKADDR)&server_addr, sizeof(server_addr)))
	{
		printf("Bind server host failed!");
		exit(1);
	}

	// listen
	if (SOCKET_ERROR == listen(ServerSocket, 10))
	{
		printf("Listen failed!");
		exit(1);
	}

	while (true)
	{
		printf("\nListening ... \n\n");
		sockaddr_in addrClient;
		int nClientAddrLen = sizeof(addrClient);
		SOCKET ClientSocket = accept(ServerSocket, (sockaddr*)&addrClient, &nClientAddrLen);
		if (SOCKET_ERROR == ClientSocket)
		{
			printf("Accept failed!");
			break;
		}
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		if (recv(ClientSocket, buffer, BUFFER_SIZE, 0) < 0)
		{
			printf("Recvive data failed!");
			break;
		}

		printf("Recv data : \n\n%s", buffer);

		// response

		memset(buffer, 0, BUFFER_SIZE);
		strcpy_s(buffer, sizeof(HTML), HTML);

		if (send(ClientSocket, buffer, strlen(buffer), 0) < 0)
		{
			printf("Send data failed!");
			break;
		}

		printf("Send data : \n\n%s\n", buffer);
		closesocket(ClientSocket);
	}

	closesocket(ServerSocket);
	WSACleanup();

	return 0;
}