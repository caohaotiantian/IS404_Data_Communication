#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#pragma comment(lib,"Ws2_32.lib")

#define DEFAULT_PORT "80"
#define DEFAULT_BUFLEN 1024

using namespace std;

int main(int argc, const char* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: WebClient <website address>" << endl;
		return 1;
	}
	// Initialize Winsock
	WORD wVersion = MAKEWORD(2, 2);
	WSAData wsaData;
	int iResult;
	iResult = WSAStartup(wVersion, &wsaData);
	if (iResult != 0)
	{
		cout << "WSAStartup failed: " << iResult << endl;
		return 0;
	}

	// Declare an addrinfo object that contains a sockaddr structure
	struct addrinfo
		* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	// Create a SOCKET object called ClientSocket
	SOCKET ClientSocket = INVALID_SOCKET;
	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (ClientSocket == INVALID_SOCKET) {
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ClientSocket);
		ClientSocket = INVALID_SOCKET;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ClientSocket == INVALID_SOCKET) {
		cout << "Unable to connect to server!\n";
		WSACleanup();
		return 1;
	}

	/*const char* sendbuf =
		"GET / HTTP/1.1\nAccept-Encoding: gzip, deflate, br\n\
		Accept - Language: zh - CN, zh; q = 0.9, en; q = 0.8, en - GB; q = 0.7, en - US; q = 0.6\n\n";*/

	const char* sendbuf =
		"GET / HTTP/1.1\n\n";

	// Send an initial buffer
	iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		cout << "send failed: " << WSAGetLastError() << endl;
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	cout << "Bytes Sent: " << iResult << endl;
	// shutdown the connection for sending since no more data will be sent
   // the client can still use the ClientSocket for receiving data
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		cout << "shutdown failed: " << WSAGetLastError() << endl;
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	char recvbuf1[DEFAULT_BUFLEN];
	char recvbuf2[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult1, iResult2;

	// Receive data until the server closes the connection
	do {
		memset(recvbuf1, 0, recvbuflen);
		memset(recvbuf2, 0, recvbuflen);
		iResult1 = recv(ClientSocket, recvbuf1, recvbuflen, 0);
		iResult2 = recv(ClientSocket, recvbuf2, recvbuflen, 0);
		if (iResult1 > 0 and iResult2 > 0) {
			if (sizeof(recvbuf1) != recvbuflen)
			{
				recvbuf1[iResult1] = '\0';
			}
			if (sizeof(recvbuf2) != recvbuflen)
			{
				recvbuf1[iResult2] = '\0';
			}
			cout << "Bytes received: " << iResult1 << endl;
			cout << recvbuf1 << endl;
			cout << "Bytes received: " << iResult2 << endl;
			cout << recvbuf2 << endl;
		}
		else if (iResult1 > 0 and iResult2 == 0) {
			if (sizeof(recvbuf1) != recvbuflen)
			{
				recvbuf1[iResult1] = '\0';
			}
			cout << "Bytes received: " << iResult1 << endl;
			cout << recvbuf1 << endl;
			cout << "Connection closed\n";
		}
		else if (iResult1 == 0)
		{
			cout << "Connection closed\n";
		}
		else {
			cout << "recv failed: " << WSAGetLastError() << endl;
		}
	} while (iResult1 > 0 and iResult2 > 0);
	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}