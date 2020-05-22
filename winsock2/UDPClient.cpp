#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define IP "127.0.0.1"
#define PORT 8890
#define BUF_SIZE 1024

class CClient
{
public:
	CClient();
	void RecMsg();
	void SendMsg(const char sendBuf[]);
	~CClient();
private:
	SOCKET m_sockClient;
	sockaddr_in m_TargetServer;
};

CClient::CClient()
{
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		std::cout << "Initialize WSA failed" << std::endl;
		return;
	}

	m_sockClient = socket(AF_INET, SOCK_DGRAM, 0);
	m_TargetServer.sin_addr.S_un.S_addr = inet_addr(IP);
	m_TargetServer.sin_family = AF_INET;
	m_TargetServer.sin_port = htons(PORT);

	if (m_sockClient == -1)
	{
		std::cout << "Create socket failed!" << std::endl;
		WSACleanup();
	}
	else
	{
		//发送信息与服务器建立连接(必须加)
		sendto(m_sockClient, "hello server", strlen("hello server"), 0, (sockaddr*)&m_TargetServer, sizeof(m_TargetServer));
	}
}

void CClient::SendMsg(const char sendBuf[])
{
	sendto(m_sockClient, sendBuf, strlen(sendBuf), 0, (sockaddr*)&m_TargetServer, sizeof(m_TargetServer));
}
void CClient::RecMsg()
{
	char recBuf[BUF_SIZE];

	while (1)
	{
		int len = recvfrom(m_sockClient, recBuf, sizeof(recBuf), 0, 0, 0);
		if (len > 0)
		{
			recBuf[len] = '\0';
			std::cout << "Server say: " << recBuf << std::endl;
		}
	}
}
CClient::~CClient()
{
	closesocket(m_sockClient);
	WSACleanup();
}

int main(int argc, const char* argv[])
{
	CClient udp_client = CClient::CClient();

	thread RecProc(&CClient::RecMsg, &udp_client);

	while (1)
	{
		string content;
		getline(cin, content);
		udp_client.SendMsg(content.c_str());
	}

	RecProc.join();
	return 0;
}