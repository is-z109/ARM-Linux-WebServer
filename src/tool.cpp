#include"tool.h"
using namespace std;
string getip(SOCKET client,string &IP)
{
	sockaddr_in ipaddr{};
	int ret;
	socklen_t ipaddr_len = sizeof(ipaddr);
	ret = getpeername(client, (struct sockaddr*)&ipaddr, &ipaddr_len);
	if (ret == -1)
	{
		return "getpeername false";
	}
	char ip[INET_ADDRSTRLEN] = { 0 };
	if (inet_ntop(AF_INET, &ipaddr.sin_addr, ip, sizeof(ip)) == NULL)
	{
		return "inet_ntop false";
	}
	IP = ip;
	return ip;
}