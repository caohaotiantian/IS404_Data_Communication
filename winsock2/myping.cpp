#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <time.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#define BUFSIZE 1024
#define IP_BUFFER_LENGTH 46

// 统计接收到的数据包的数量
static int recvPkgCount = 0;
// 统计收发时间
static int minTime = 0, maxTime = 0, sumTime = 0;
// 设置一个标志位记录是否初始化时间
static int time_flag = 0;

// 将主机名转换为 IP 地址
char* gethostaddress(char* hostname)
{
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf_s("WSAStartup failed: %d\n", iResult);
		return nullptr;
	}
	addrinfo* result = NULL;
	addrinfo* ptr = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(hostname, 0, &hints, &result) != 0)
	{
		printf_s("getaddrinfo failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return nullptr;
	}

	sockaddr_in* sockaddr_ipv4;

	char ipstringbuffer[IP_BUFFER_LENGTH];

	switch (result->ai_family)
	{
	case AF_INET:
		printf_s("AF_INET (IPv4)\n");
		sockaddr_ipv4 = (sockaddr_in*)result->ai_addr;
		inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipstringbuffer, sizeof(ipstringbuffer));
		printf_s("IPv4 address: %s\n", ipstringbuffer);
		return ipstringbuffer;
		break;

	default:
		printf_s("type: %ld\n", result->ai_family);
		printf_s("本例地址转换仅支持 IPv4\n");
		break;
	}
	switch (result->ai_socktype)
	{
	case 0:
		printf_s("Unspecified\n");
		break;
	case SOCK_STREAM:
		printf_s("SOCK_STREAM (stream)\n");
		break;
	case SOCK_DGRAM:
		printf_s("SOCK_DGRAM (datagram)\n");
		break;
	case SOCK_RAW:
		printf_s("SOCK_RAW (raw)\n");
		break;
	case SOCK_RDM:
		printf_s("SOCK_RDM (reliable message datagram)\n");
		break;
	case SOCK_SEQPACKET:
		printf_s("SOCK_SEQPACKET (pseudo-stream packet)\n");
		break;
	default:
		printf_s("Other %ld\n", result->ai_socktype);
		break;
	}
	printf_s("\tProtocol: ");
	switch (result->ai_protocol)
	{
	case 0:
		printf_s("Unspecified\n");
		break;
	case IPPROTO_TCP:
		printf_s("IPPROTO_TCP (TCP)\n");
		break;
	case IPPROTO_UDP:
		printf_s("IPPROTO_UDP (UDP) \n");
		break;
	default:
		printf_s("Other %ld\n", result->ai_protocol);
		break;
	}
	return nullptr;
}

// 将 IP 地址转换为主机名
char* gethostname(char* ip)
{
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		return nullptr;
	}
	DWORD dwRetval;
	sockaddr_in saGHN;
	u_short port = 27015;
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];
	saGHN.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &saGHN.sin_addr.S_un.S_addr);
	saGHN.sin_port = htons(port);
	dwRetval = getnameinfo((sockaddr*)&saGHN, sizeof(sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
	if (dwRetval != 0)
	{
		// printf("getnameinfo failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return nullptr;
	}
	else
	{
		// printf("getnameinfo returned hostname = %s\n", hostname);
		WSACleanup();
		return hostname;
	}
}

// 定义 ICMP 头部消息
typedef struct icmp_header
{
	// 构造消息类型
	unsigned char icmp_type;
	// 代码
	unsigned char icmp_code;
	// 校验和
	unsigned short icmp_checksum;
	// 唯一标识
	unsigned short icmp_id;
	// 序列号
	unsigned short icmp_sequence;
} icmp_header;

// 计算校验和
unsigned short checksum(struct icmp_header* pICMP, int len)
{
	long sum = 0;
	unsigned short* tmp = (unsigned short*)pICMP;
	while (len > 0)
	{
		sum += *(tmp++);
		if (sum & 0x80000000)
		{
			sum = (sum & 0xffff) + (sum >> 16);
		}
		len -= 2;
	}
	if (len)
	{
		sum += (unsigned short)*(unsigned char*)tmp;
	}
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return (unsigned short)~sum;
}

// ping 函数
int ping(char* DestIP)
{
	// printf_s("目标地址 = %s\n", DestIP);
	// 初始化 winsock2
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf_s("Failed to initialize winsock2: %d\n", WSAGetLastError());
		return 1;
	}

	// 创建一个 socket
	SOCKET s = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);

	// 设置超时
	int timeout = 1000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	// 设置目的地址
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.S_un.S_addr = inet_addr(DestIP);
	dest_addr.sin_port = htons(0);

	// 构造 ICMP 数据包
	char buff[sizeof(icmp_header) + 32] = { 0 };
	char icmp_data[32] = { 0 };
	icmp_header* pICMP = (icmp_header*)buff;
	pICMP->icmp_type = 0x08;
	pICMP->icmp_code = 0;
	pICMP->icmp_id = (USHORT)::GetCurrentProcessId();
	pICMP->icmp_sequence = 0;
	pICMP->icmp_checksum = 0;

	// 构造一条消息，可以任意
	memcpy((buff + sizeof(icmp_header)), "This is a test message,have fun", 32);
	pICMP->icmp_checksum = checksum((struct icmp_header*)buff, sizeof(buff));

	// 记录起始时间
	DWORD start = GetTickCount();

	// 发送报文
	char recvBuff[BUFSIZE];
	iResult = sendto(s, buff, sizeof(buff), 0, (SOCKADDR*)&dest_addr, sizeof(SOCKADDR));

	// 构造目标地址
	sockaddr_in from_addr;

	// 设置成功与否的标志位
	int flag = 0;
	// 统计发起的请求数量
	int count = 0;

	// 处理收到的报文
	while (true)
	{
		// 尝试请求主机，如果没有在规定的请求次数内收到回应则判定请求失败
		if (count++ > 5)
		{
			flag = 1;
			break;
		}

		// 接收消息
		memset(recvBuff, 0, BUFSIZE);
		int Len = sizeof(from_addr);
		iResult = recvfrom(s, recvBuff, MAXBYTE, 0, (SOCKADDR*)&from_addr, &Len);

		// 接收到来自目标的报文
		if (strcmp(inet_ntoa(from_addr.sin_addr), DestIP) == 0)
		{
			recvPkgCount++;
			break;
		}
	}

	// 记录结束时间
	DWORD end = GetTickCount();

	// 计算花费的时间
	DWORD time = end - start;

	// 最大/最小时间还没初始化
	if (time_flag == 0)
	{
		maxTime = time;
		minTime = time;
		time_flag = 1;
	}
	// 已经初始化，则记录最大/最小时间
	else
	{
		if (time > maxTime)
		{
			maxTime = time;
		}
		if (time < minTime)
		{
			minTime = time;
		}
	}
	// 记录总时间
	sumTime += time;

	// 如果失败，打印信息
	if (flag)
	{
		printf_s("请求超时\n");
		return 0;
	}

	// IP 头部的第一个字节，前 4 位表示 IP 协议的版本号，后 4 位表示 IP 的头部长度
	char ipInfo = recvBuff[0];
	// IPv4 头部的前 9 个字节为 TTL 的值
	char ttl = recvBuff[8];
	// 打印相关信息
	// printf_s("ipInfo = %x\n", ipInfo);

	// 判断 IP 协议版本
	int ipVersion = ipInfo >> 4;
	int ipHeadLen = ((char)(ipInfo << 4) >> 4) * 4;
	// IPv4
	if (ipVersion == 4)
	{
		// 网络字节序是大端模式，高位字节存放在低地址
		// 但是本地的字节序是小端模式，需要做一个转换
		icmp_header* icmp_get = (icmp_header*)(recvBuff + ipHeadLen);
		// 校验和是两个字节，需要做一个转换
		// unsigned short checksum = ntohs(icmp_get->icmp_checksum);
		// 打印相关信息
		// printf_s("type = %d\nchecksum = %x\n", icmp_get, checksum);

		// 显示应答报文
		if (icmp_get->icmp_type == 0)
		{
			printf_s("来自 %s 的回复：字节=32 时间=%2dms TTL=%d \n", DestIP, time, ttl);
		}
		else
		{
			printf_s("请求超时 type = %d\n", icmp_get->icmp_type);
		}
	}
	else
	{
		printf_s("本程序目前仅支持 IPv4 地址\n");
	}
	return 0;
}

// 主函数
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf_s("Usage:\nmyping.exe <IP address> [ <max send count> ]\n");
		return 1;
	}
	char* address = gethostaddress(argv[1]);
	char ip[32];
	strcpy_s(ip, sizeof(ip), address);

	// 这里可以通过 IP 地址获取主机名
	// 但是由于之前实验中相关函数的设计缺陷，此功能不予启用
	/*
	char host[64];
	strcpy_s(host, sizeof(host), gethostname(ip));
	printf_s("\n获取主机名如下：\n");
	printf_s(host);
	printf_s("\n");
	*/

	if (strcmp(address, argv[1]) == 0)
	{
		printf_s("\n正在 Ping %s 具有 32 字节的数据：\n", argv[1]);
	}
	else
	{
		printf_s("\n正在 Ping %s \[%s\] 具有 32 字节的数据：\n", argv[1], ip);
	}
	int i = 0;
	int maxSendCount;
	if (argc == 2)
	{
		maxSendCount = 4;
	}
	else
	{
		maxSendCount = atoi(argv[2]);
	}
	while (i < maxSendCount)
	{
		int result = ping((char*)ip);
		Sleep(500);
		i++;
	}
	if (strcmp(address, argv[1]) == 0)
	{
		printf_s("\n%s 的 Ping 统计信息：\n", argv[1]);
	}
	else
	{
		printf_s("\n%s 的 Ping 统计信息：\n", ip);
	}

	printf_s("    数据包：已发送 = %d，已接收 = %d，丢失 = %d (%d%%) 丢失\n", i, recvPkgCount, i - recvPkgCount, (i - recvPkgCount) * 100 / i);
	if (i - recvPkgCount >= maxSendCount)
	{
		return 0;
	}
	printf_s("往返时间统计结果：\n");
	printf_s("    最短 = %dms，最长 = %dms，平均 = %dms\n", minTime, maxTime, sumTime / recvPkgCount);
	return 0;
}