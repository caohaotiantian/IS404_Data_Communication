#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define NI_MAXSERV 32
#define NI_MAXHOST 1024

int main(int argc, char const* argv[])
{
	// create a WSADATA object called wsaData
	WSADATA wsaData;

	// call WSAStartup and return its value as an integer and check for errors
	int iResult;

	// initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	char host_name[128];
	// get the host information
	if (argc < 2)
	{
		if (gethostname(host_name, sizeof(host_name)) != 0)
		{
			printf("get host information failed\n");
			WSACleanup();
			return 1;
		}
		else
		{
			printf("Host information: %s\n", host_name);
			WSACleanup();
			return 0;
		}
	}
	else
	{
		if (argc > 3)
		{
			printf("Only 1 parameter expected, but got %d\n", argc - 1);
			printf("Usage: \n");
			printf("[1]: %s\n", argv[0]);
			printf("[2]: %s <IPv4 address>\n", argv[0]);
			printf("Example #1:\n");
			printf("gethostname\n");
			printf("Example #2:\n");
			printf("gethostname 192.168.0.154");
			return 1;
		}
		else
		{
			DWORD dwRetval;
			sockaddr_in saGHN;
			u_short port = 27015;
			char hostname[NI_MAXHOST];
			char servInfo[NI_MAXSERV];
			saGHN.sin_family = AF_INET;
			inet_pton(AF_INET, argv[1], &saGHN.sin_addr.S_un.S_addr);
			saGHN.sin_port = htons(port);
			dwRetval = getnameinfo((sockaddr*)&saGHN, sizeof(sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
			if (dwRetval != 0)
			{
				printf("getnameinfo failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				return 1;
			}
			else
			{
				printf("getnameinfo returned hostname = %s\n", hostname);
				WSACleanup();
				return 0;
			}

		}
	}
	return 0;
}