#pragma once
#include<unordered_map>
#include<vector>
#include<string>
#include<mutex>
#include<fstream>
using namespace std;
class user_login {
private:
	unordered_map<string, string> username;
	mutex login_lock;
	user_login()
	{		username.clear();
		ifstream infile("u_name.txt");
		if (!infile.is_open())
		{
			return;
		}
		string usn, num;
		while (infile >> usn >> num)
		{
			username[usn] = num;
		}
		infile.close();
	}
public:
	user_login(const user_login&) = delete;
	user_login& operator=(const user_login&) = delete;
	int insert(string user_name, string user_pwd)
	{
		unique_lock<mutex>lock(login_lock);
		if (username.count(user_name) == 1)
		{
			lock.unlock();
			return -1;
		}
		username.insert({ user_name,user_pwd });
		return 0;
	}
	int remove(string user_name, string user_pwd)
	{
		unique_lock<mutex>lock(login_lock);
		if (username.count(user_name) == 0)
		{
			lock.unlock();
			return -1;
		}
		username.erase(user_name);
		return 0;
	}
	int rebuild(string user_name, string user_pwd,string newpwd)
	{
		remove(user_name, user_pwd);
		unique_lock<mutex>lock(login_lock);
		username.insert({ user_name,newpwd });
		return 0;
	}
	bool look(string user_name, string user_pwd)
	{
		unique_lock<mutex>lock(login_lock);
		if (username.count(user_name) == 0)
		{
			return false;
		}
		if (user_pwd != username[user_name])
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	static user_login& instance()
	{
		static user_login usermodel;
		return usermodel;
	}
	~user_login()
	{
		unique_lock<mutex>lock(login_lock);
		ofstream outfile("u_name.txt", ios::trunc);
		if (!outfile.is_open())
		{
			cout << "д���ļ�ʧ�ܣ�(n)" << endl;
			return;
		}
		for (const auto& pair : username)
		{
			outfile << pair.first << " " << pair.second << "\n";
		}
		outfile.close();
		cout << "д��ɹ�?" << endl;
	}
};
