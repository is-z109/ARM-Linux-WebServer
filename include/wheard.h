#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>     // 用于 close() 函数
#include <fcntl.h>      // 用于设置非阻�???
#include <errno.h>      // 用于查看错�??�???
#include <cstring>
#include<fstream>
#include<filesystem>
#include<iostream>
#include<sstream>
#include<string>
#include<thread>
#include<chrono>
#include<vector>
#include <sys/uio.h>
#include"httprequest.h"
#include"log.h"
#include"Login.h"
#include"tool.h"
#include"GPIO.h"
#include"mqueue.h"
#include"json.hpp"
#include"updata.h"
mutex json_lock;
extern data_b stm32data;
using json = nlohmann::json;
string webroot = "/home/lyra/novel/";
GPIO light("0"),tempreture("1"),bright("41");
json j;
using namespace std;
typedef int SOCKET;
struct jsonmessage{
	double tempre;
	double lig;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jsonmessage,tempre,lig);
class socketlisten {                    //????listen?????
	struct sockaddr_in addr;
	int listensocket;
public:
	socketlisten(int port)
	{
		 listensocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listensocket == -1) {
          perror("socket error"); // ��ӡ������Ϣ
          return;
	    }
		memset(&addr,0,sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY); // ����������������
        addr.sin_port = htons(port);
		int opt = 1;
    setsockopt(listensocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		int ret = bind(listensocket, (struct sockaddr*)&addr, sizeof(addr));
        if (ret == -1) {
          perror("bind error");
          return ;
		}
        ret=listen(listensocket,5);
		if(ret==-1)
		{
			perror("listen error");
			return;
		}
	}
	SOCKET getsocket()
	{
		return listensocket;
	}
};


void Not_Found404(SOCKET clientsocket)           //�ͳ�404����
{
	string body404("<html><body><h1>404: File Not Found</h1></body></html>");
	int len404 = body404.length();
	string slen404 = to_string(len404);
	string literally404 = "HTTP/1.1 404 Not Found\r\nContent-Type:text/html\r\nContent-Length:" + slen404 + "\r\nConnection: close\r\n\r\n" + body404;
	int lastlen404 = literally404.length();
	send(clientsocket, literally404.data(), lastlen404, 0);
	close(clientsocket);
}
void Bad_Request400(SOCKET clientsocket)        //�ͳ�400����
{
	string body400("<html><body><h1>400: Bad Request</h1></body></html>");
	int len400 = body400.length();
	string slen400 = to_string(len400);
	string literally400 = "HTTP/1.1 400 Bad Request\r\nContent-Type:text/html\r\nContent-Length:" + slen400 + "\r\nConnection: close\r\n\r\n" + body400;
	int lastlen400 = literally400.length();
	send(clientsocket, literally400.data(), lastlen400, 0);
	close(clientsocket);
}
string make_card(string filename,string cover_name)
{
	string temp;
	temp = "<div class=\"book-card\">\n  <img src = \"cover/";
	temp += cover_name;
	temp += ".jpg\" class = \"book-cover\">\n  <div class=\"book-info\">\n   <a href=\"";
	temp += filename;
	temp += "\" class=\"book-title\">";
	temp += filename;
	temp += "</a>\n  </div>\n</div>";
	return temp;
}
void Content(string webroot, SOCKET clientsocket)        //�ͳ������б�html
{
	string webhtml = "/home/lyra/novel/index.html";
	ifstream infile;
	infile.open(webhtml);
	stringstream novelstream;
	novelstream << infile.rdbuf();
	string index;
	index = novelstream.str();
	string filename,temp,cover_name;
	string allcards("");
	for (filesystem::directory_iterator a(webroot); a != filesystem::directory_iterator(); ++a)
	{
		filename = a->path().filename().u8string();
		if (filename == "index.html" || filename == "style.css"||filename=="cover"||filename=="login.html"|| filename == "register.html")
			continue;
		allcards += make_card(filename, a->path().stem().u8string());
	}
	int addr,len;
	addr=index.find("zhanweifuwei");
	temp = index.substr(0, addr);
	temp += allcards;
	temp += index.substr(addr + 12, index.size());
	len = temp.size();
	string hdlen = to_string(len);
	string literally = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: keep-alive\r\nContent-Length: " + hdlen + "\r\n\r\n" +temp;
	send(clientsocket, literally.data(), literally.length(), 0);
	close(clientsocket);

}
bool IsLoggedIn(HttpRequest& request) {
	if (request.headers.count("Cookie")) {
		string cookie = request.headers["Cookie"];
		// �򵥼��COOKIE
		if (cookie.find("is_login=true") != string::npos) {
			return true;
		}
	}
	return false;
}
bool sendfile(SOCKET client, string& filename)
{
	ifstream file(filename, ios::binary);
	if (!file)
		return false;
	char buff[4096];
	while (!file.eof())
	{
		file.read(buff, sizeof(buff));
		int len = file.gcount();
		send(client, buff, len, 0);
	}
	file.close();
	return true;
}
void sendjson(SOCKET client)
{
	jsonmessage message;
	message.tempre=(tempreture.GPIO_GetInput()=='1'? 0.0:1.0);
	message.lig=(bright.GPIO_GetInput()=='1'? 0.0:1.0);
	j=message;
	string js=j.dump();
	//cout<<"js: "<<js<<endl;
	struct iovec iov[2];
	string http="HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(js.length()) + "\r\n" 
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n"; 

    iov[0].iov_base = (void*)http.c_str();
    iov[0].iov_len = http.length();
    iov[1].iov_base = (void*)js.c_str();
    iov[1].iov_len = js.length();
	writev(client, iov, 2);
	close(client);
}
void sendusart(SOCKET client)
{
	jsonmessage message;
	unique_lock<mutex>lock(json_lock);
	message.tempre=stm32data.temperature;
	message.lig=stm32data.humidity;
	j=message;
	string js=j.dump();
	//cout<<"js: "<<js<<endl;
	struct iovec iov[2];
	string http="HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(js.length()) + "\r\n" 
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n"; 

    iov[0].iov_base = (void*)http.c_str();
    iov[0].iov_len = http.length();
    iov[1].iov_base = (void*)js.c_str();
    iov[1].iov_len = js.length();
	writev(client, iov, 2);
	close(client);
}
void handleclient(SOCKET clientsocket)
{
	HttpRequest request;
	request.init();
	char buffer[4096];
	int answer;
	HttpCode ret = NO_REQUEST;
	string content_type = "text/plain", disposition = "attachment";
	string filename = "";
	string localpath = webroot;
	long long filesize;

	// 1. ���ղ����� HTTP ����ͷ
	while (1)
	{
		answer = recv(clientsocket, buffer, 4096, 0);
		if (answer == 0)
		{
			close(clientsocket);
			return;
		}
		if (answer > 0)
			ret = request.parse(buffer, answer);
		if (answer < 0)
		{
			close(clientsocket);
			return;
		}
		if (ret == GET_REQUEST)
			break;
		else if (ret == BAD_REQUEST)
		{
			Bad_Request400(clientsocket);
			close(clientsocket);
			return;
		}
		else if (ret == NO_REQUEST)
		{
			continue;
		}
	}
	string literally;

	// 2. Ȩ��У����δ��¼״̬����
	if (!IsLoggedIn(request))
	{
		// 2.1 �����¼ҳ��???
		if (request.path() == "/login.html"|| request.path() == "/register.html")
		{
			filename = request.path();
			// ��ȫ��ȡ��·����ͷ�� "/"
			if (!filename.empty() && filename[0] == '/') {
				filename = filename.substr(1);
			}

			if (filename.find(".html") != string::npos)
			{
				content_type = "text/html";
				disposition = "inline";
			}
			else if (filename.find(".css") != string::npos)
			{
				content_type = "text/css";
				disposition = "inline";
			}
			localpath += filename;

			ifstream file(localpath, ios::binary | ios::ate);
			if (!file.is_open())
			{
				string _404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
				send(clientsocket, _404.data(), _404.length(), 0);
				close(clientsocket);
				return;
			}

			filesize = file.tellg();
			file.seekg(0);
			literally = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "; charset=utf-8\r\nContent-Length: " + to_string(filesize) + "\r\nContent-Disposition: " + disposition + "\r\nConnection: close\r\n\r\n";

			send(clientsocket, literally.data(), literally.length(), 0);
			sendfile(clientsocket, localpath);

			close(clientsocket);
			return; // ���� return
		}

		// 2.2 ������¼ POST �ύ
		if (request.method() == "POST" && request.path() == "/login")
		{
			bool is_post = user_login::instance().look(request.getusername(), request.getpassword());
			if (is_post)
			{
				string response = "HTTP/1.1 302 Found\r\nLocation: /\r\nSet-Cookie: is_login=true; Path=/\r\nContent-Length: 0\r\n\r\n";
				send(clientsocket, response.data(), response.length(), 0);
				LOG::instance()->write_log(clientsocket, request.getusername(), "Login");
				close(clientsocket);
				return;
			}
			else
			{
				string _401 = "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\nContent-Length: 18\r\n\r\nname or pwd error!";
				send(clientsocket, _401.data(), _401.length(), 0);
				close(clientsocket);
				return;
			}
		}
		if (request.method() == "POST" && request.path() == "/register")
		{
			int utemp;
			utemp=user_login::instance().insert(request.getusername(), request.getpassword());
			if (utemp == -1)
			{
				cout << "�û����Ѵ���";
				string _401 = "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\nContent-Length: 18\r\n\r\nname or pwd error!";
				send(clientsocket, _401.data(), _401.length(), 0);
				close(clientsocket);
				return;
			}
		}

		// 2.3 ����δ��Ȩ����ͳһ�ض��򵽵�¼ҳ
		string response1 = "HTTP/1.1 302 Found\r\nLocation: /login.html\r\nSet-Cookie: is_login=false; Path=/\r\nContent-Length: 0\r\n\r\n";
		send(clientsocket, response1.data(), response1.length(), 0);
		LOG::instance()->write_log(clientsocket, request.path());
		close(clientsocket);
		return;
	}
	// 3. �ѵ�¼״̬���� (����ҵ����?�ļ�)
	else
	{
		if (request.method() == "POST" && request.path() == "/login")
        {
	bool is_post = user_login::instance().look(request.getusername(), request.getpassword());
	if (!is_post)
	{
		string _401 = "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\nContent-Length: 18\r\n\r\nname or pwd error!";
		send(clientsocket, _401.data(), _401.length(), 0);
		close(clientsocket);
		return;
	}
}
if (request.path() == "/" || request.path().empty()||(IsLoggedIn(request)&&request.method()=="POST"&&request.path() == "/login"))
		{
			Content(webroot, clientsocket);
			return; // ���� Content ���ѽӹ���Ӧ���ر��� socket
		}

if(request.path()=="/on"||request.path()=="/off"||request.path()=="/message")
{
	if(request.path()=="/on")
	{
	  mqueue::instance().insert([](){usleep(50000);
	 light.GPIO_direction("out");
	 light.GPIO_Write('1');
	 return;});
	 close(clientsocket);
	}
	if(request.path()=="/off")
	{
	mqueue::instance().insert([](){usleep(50000);
	 light.GPIO_direction("out");
	 light.GPIO_Write('0');
	 return;});
	 close(clientsocket);
	}
	if(request.path()=="/message")
	{
		sendusart(clientsocket);
	}
	return ;
}
		// 3.2 ��������ľ��?�ļ�·��
		filename = request.path();
		if (!filename.empty() && filename[0] == '/')
		{
			filename = filename.substr(1);
		}
		localpath += filename;

		// 3.3 ���ݺ�׺ʶ���ļ�����
		if (filename.find(".jpg") != string::npos || filename.find(".jpeg") != string::npos)
		{
			content_type = "image/jpeg";
			disposition = "inline";
		}
		else if (filename.find(".html") != string::npos)
		{
			content_type = "text/html";
			disposition = "inline";
		}
		else if (filename.find(".css") != string::npos)
		{
			content_type = "text/css";
			disposition = "inline";
		}
		else
		{
			content_type = "application/octet-stream";
			disposition = "attachment";
		}

		// 3.4 ���Դ��ļ������� 404 ����
		ifstream file(localpath, ios::binary | ios::ate);
		if (!file.is_open())
		{
			string _404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
			send(clientsocket, _404.data(), _404.length(), 0);
			close(clientsocket);
			return;
		}

		// 3.5 �����С������??? HTTP 200 ����
		filesize = file.tellg();
		file.seekg(0);

		literally = "HTTP/1.1 200 OK\r\n"
			"Content-Type: " + content_type + "; charset=utf-8\r\n"
			"Content-Length: " + to_string(filesize) + "\r\n"
			"Content-Disposition: " + disposition + "\r\n"
			"Connection: close\r\n\r\n";

		// 3.6 ������Ӧ
		send(clientsocket, literally.data(), literally.length(), 0);
		sendfile(clientsocket, localpath);

		close(clientsocket);
		return;
	}
}
