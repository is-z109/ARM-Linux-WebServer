#include <sys/epoll.h>
#include"wheard.h"
#include"theardpool.h"
#include"log.h"
#include"timer.h"
#include"Login.h"
#include"mqueue.h"
using namespace std;
std::atomic<bool> server_running{ true };
typedef int SOCKET;
extern GPIO light;
data_b stm32data;
// 2. �źŴ��������������Ǵ��� int �����Ĺ̶�ǩ����
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
int main()
{
	int choose;
	setbuf(stdout, NULL);
	LOG::instance();
	socketlisten monitor(12000);
	setnonblocking(monitor.getsocket());
	struct epoll_event ev;
	ev.data.fd=monitor.getsocket();
	ev.events=EPOLLIN | EPOLLET;
	int epollfd=epoll_create1(0);
	if(epollfd==-1)
	{
		perror("epoll_create1");
		return -1;
	}
	epoll_ctl(epollfd,EPOLL_CTL_ADD,monitor.getsocket(),&ev);
	SOCKET clientsocket;
	sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	int fd_count;
	epoll_event client[1024];
	threadpool pool(4);
	light.GPIO_Init();
	tempreture.GPIO_Init();
	tempreture.GPIO_direction("in");
	bright.GPIO_Init();
	bright.GPIO_direction("in");
	mqueue::instance();
	thread poll(updata,ref(stm32data));
	poll.detach();
	while (1)
	{
		fd_count=epoll_wait(epollfd,client,1024,-1);
		for(int i=0;i<fd_count;i++)
		{
			if(client[i].data.fd==monitor.getsocket())
			{
				while(1)
				{
				SOCKET clientsocket=accept(monitor.getsocket(),(sockaddr*)&client_addr,&client_addr_size);
				if(clientsocket<0)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					   break;
					else
					  return -1;

				}
				pool.enqueue([clientsocket](){handleclient(clientsocket);});
				}
			}
			
		}
	}
	return 0;
}