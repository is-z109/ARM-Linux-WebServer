#pragma once
#include<string>
#include<map>
#include<unordered_map>
using namespace std;
enum CheckState{ CHECK_STATE_REQUESTLINE, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };//请求行，头部，正文
enum HttpCode{NO_REQUEST,GET_REQUEST,BAD_REQUEST};//数据不全，解析成功，请求错误
class HttpRequest {
	string buffer;
	CheckState check_state;
	string method_, url_, version_;
	unsigned int content_len;
	string body_;
	unordered_map<string, string> post;
	HttpCode parse_request_line(string& text);//解析请求行 (GET /index.html HTTP/1.1)
	HttpCode parse_headers(string& text);//解析头部
	HttpCode parse_content(string& text);//解析正文
public:
	bool is_keep_live = true;
	map<string, string>headers;
	void init();//初始化函数
	void ParseFromUrlencoded_(string body);//提取账密;
	HttpCode parse(const char* text, int len);//解析
	string path()const { return url_; }//获取url
	string method()const { return method_; }//获取请求头
	void check_keeplive()
	{
		if (headers.find("Connection") != headers.end())
		{
			if (headers["Connection"] == "close")
				is_keep_live = false;
		}

	}
	bool buffer_empty()
	{
		if (buffer.empty())
			return true;
		else
			return false;
	}
	string getusername()
	{
		return post["username"];
	}
	string getpassword()
	{
		return post["password"];
	}
	int getbuffersize()
	{
		return buffer.size();
	}
};
HttpCode HttpRequest::parse(const char* text, int len)
{
	buffer.append(text,len);
	while (buffer.size()>0)
	{
		if (check_state == CHECK_STATE_CONTENT)
		{
			if (HttpRequest::parse_content(buffer) == GET_REQUEST)
				return GET_REQUEST;
			else
				return NO_REQUEST;
		}
		else
		{
			unsigned int addr=buffer.find("\r\n");
			string temp;
			if (addr==string::npos)
			{
				return NO_REQUEST;
			}
			else
			{
				temp = buffer.substr(0, addr);
				buffer.erase(0, addr + 2);
			}
			switch (check_state)
			{
			case CHECK_STATE_REQUESTLINE: {
				if (HttpRequest::parse_request_line(temp) == BAD_REQUEST)
					return BAD_REQUEST;
				break;
			}
			case CHECK_STATE_HEADER: {
				HttpCode ret = HttpRequest::parse_headers(temp);
				if (ret == BAD_REQUEST)
					return BAD_REQUEST;
				if (ret == GET_REQUEST)
				{
					check_keeplive();
					return GET_REQUEST;
				}
				break;
			}
			}
		}
	}
	return NO_REQUEST;
}
void HttpRequest::init()
{
	check_state = CHECK_STATE_REQUESTLINE;
	url_.clear();
	method_.clear();
	headers.clear();
	post.clear();
	is_keep_live = true;
	content_len = 0;
}
HttpCode HttpRequest::parse_request_line(string& text)
{
	unsigned int addr;
	addr = text.find(' ');
	if (addr == string::npos)
		return BAD_REQUEST;
	method_ = text.substr(0, addr);
	text.erase(0, addr + 1);
	addr = text.find(' ');
	if (addr == string::npos)
		return BAD_REQUEST;
	url_ = text.substr(0, addr);
	text.erase(0, addr + 1);
	version_ = text;
	if (version_ != "HTTP/1.1")
		return BAD_REQUEST;
	check_state = CHECK_STATE_HEADER;
	return NO_REQUEST;
}
HttpCode HttpRequest:: parse_headers(string& text)
{
	if (text.empty())
	{
		if (headers.find("Content-Length") != headers.end() && method_ == "POST")
		{
			check_state = CHECK_STATE_CONTENT;
			return NO_REQUEST;
		}
		else
		 return GET_REQUEST;
	}
	unsigned int addr;
	string temp1;
	addr = text.find(':');
	if (addr == string::npos)
		return BAD_REQUEST;
	temp1 = text.substr(0, addr);
	text.erase(0, addr + 2);
	if (temp1 == "Content-Length")
		content_len = stoi(text);
	headers[temp1] = text;
	return NO_REQUEST;
}
HttpCode HttpRequest::parse_content(string& text)
{
	if (text.size() >=content_len)
	{
		body_ = text.substr(0,content_len);
		text.erase(0, content_len);
		ParseFromUrlencoded_(body_);
		return GET_REQUEST;
	}
	else
		return NO_REQUEST;
}
int converthex(char ch)
{
	int c;
	c = ch;
	if (c >= 48 && c <= 57)
		c -= 48;
	else if (c >= 97 && c <= 102)
		c -= 87;
	else if(c>=65&&c<=70)
	    c -= 55;
	return c;
}
void HttpRequest:: ParseFromUrlencoded_(string body)//提取账密
{
	string key,value;
	bool is_key = true;
	for (int i=0;i<body.size();i++)
	{
		switch (body[i])
		{
		case '%':
		{
			if(!is_key)
			  value = value + (char)(converthex(body[i + 1]) * 16 + converthex(body[i + 2]));
			else
				key=key+ (char)(converthex(body[i + 1]) * 16 + converthex(body[i + 2]));
			i += 2;
			break;
		}
		case '+':
		{
			if (!is_key)
				value += ' ';
			else
				key += ' ';
			break;
		}
		case '&':
		{
			is_key = true;
			post.insert({ key,value });
			key = "";
			value = "";
			break;
		}
		case '=':
		{
			is_key = false;
			break;
		}
		default:
		{
			if (!is_key)
				value += body[i];
			else
				key += body[i];
		}
		}
	}
	if(key!=""&&value!="")
	post.insert({ key,value });
}