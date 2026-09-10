#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>     // 用于 close() 函数
#include <fcntl.h>      // 用于设置非阻塞
#include <errno.h>      // 用于查看错误码
#include<string>
typedef int SOCKET;
using namespace std;
string getip(SOCKET client,string &IP);
