#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define PORT 8890
#define BUF_SIZE 1024

class CServer_UDP
{
public:
	CServer_UDP();
	void SendMsg(const char sendBuf[]);
	void RecvMsg();
	~CServer_UDP();

private:
	SOCKET m_sServer;
	struct sockaddr_in m_SocAddrClient;		//建立连接时，用于保存客户端信息
	bool m_terminal;
};
CServer_UDP::CServer_UDP() :m_terminal(false)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		std::cout << "Initialize WSA failed" << std::endl;
		return;
	}

	m_sServer = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in m_SocAddrserver;
	m_SocAddrserver.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	m_SocAddrserver.sin_family = AF_INET;
	m_SocAddrserver.sin_port = htons(PORT);

	int ret = bind(m_sServer, (sockaddr*)&m_SocAddrserver, sizeof(m_SocAddrserver));
	if (ret == -1)
	{
		std::cout << "bind failed!" << std::endl;
		WSACleanup();
	}
	else
	{
		int len_Client = sizeof(sockaddr);
		char recBuf[BUF_SIZE];
		int len = recvfrom(m_sServer, recBuf, sizeof(recBuf), 0, (sockaddr*)&m_SocAddrClient, &len_Client);
		if (len > 0)
		{
			recBuf[len] = '\0';
			std::cout << "Client say:" << recBuf << std::endl;
		}
	}
}
void CServer_UDP::SendMsg(const char sendBuf[])
{
	int ret = sendto(m_sServer, sendBuf, strlen(sendBuf), 0, (sockaddr*)&m_SocAddrClient, sizeof(m_SocAddrClient));
	if (ret == -1)
	{
		std::cout << "send failed" << std::endl;
		std::cout << GetLastError() << std::endl;
	}
}
void CServer_UDP::RecvMsg()
{
	char recBuf[BUF_SIZE];
	while (!m_terminal)
	{
		int len = recvfrom(m_sServer, recBuf, sizeof(recBuf), 0, 0, 0);
		if (len > 0)
		{
			recBuf[len] = '\0';
			std::cout << "Client say:" << recBuf << std::endl;
		}
	}
}
CServer_UDP::~CServer_UDP()
{
	closesocket(m_sServer);
	WSACleanup();
}

int main()
{
	CServer_UDP server_UDP;

	thread recProc(&CServer_UDP::RecvMsg, &server_UDP);

	while (1)
	{
		string content;
		getline(cin, content);
		server_UDP.SendMsg(content.c_str());
	}
	return 0;
}