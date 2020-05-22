#include <WinSock2.h>
#include <stdio.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define IP_BUFFER_LENGTH 46

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

	// declare and initialize variables
	addrinfo* result = NULL;
	addrinfo hints;

	if (argc != 3)
	{
		printf("Usage: %s <hostname> <servicename>\n", argv[0]);
		printf("Example:\n%s www.baidu.com 0\n", argv[0]);
		return 1;
	}
	else
	{
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		if (getaddrinfo(argv[1], argv[2], &hints, &result) != 0)
		{
			printf("getaddrinfo failed with error: %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		else
		{
			printf("getaddrinfo success\n");
		}

		sockaddr_in* sockaddr_ipv4;

		char ipstringbuffer[IP_BUFFER_LENGTH];

		int i = 1;

		switch (result->ai_family)
		{
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			sockaddr_ipv4 = (sockaddr_in*)result->ai_addr;
			inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipstringbuffer, sizeof(ipstringbuffer));

			printf("\tIPv4 address: %s\n", ipstringbuffer);
			break;

		default:
			printf("type: %ld\n", result->ai_family);
			printf("本例地址转换仅支持 IPv4\n");
			break;
		}
		switch (result->ai_socktype)
		{
		case 0:
			printf("Unspecified\n");
			break;
		case SOCK_STREAM:
			printf("SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			printf("SOCK_DGRAM (datagram)\n");
			break;
		case SOCK_RAW:
			printf("SOCK_RAW (raw)\n");
			break;
		case SOCK_RDM:
			printf("SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			printf("Other %ld\n", result->ai_socktype);
			break;
		}
		printf("\tProtocol: ");
		switch (result->ai_protocol)
		{
		case 0:
			printf("Unspecified\n");
			break;
		case IPPROTO_TCP:
			printf("IPPROTO_TCP (TCP)\n");
			break;
		case IPPROTO_UDP:
			printf("IPPROTO_UDP (UDP) \n");
			break;
		default:
			printf("Other %ld\n", result->ai_protocol);
			break;
		}
	}

	WSACleanup();
	return 0;
}