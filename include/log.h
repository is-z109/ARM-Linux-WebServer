#pragma once
#include<condition_variable>
#include<thread>
#include<deque>
#include<mutex>
#include<fstream>
#include<chrono>
#include<sstream>
#include<iomanip>
#include<ctime>
using namespace std;
typedef int SOCKET;
template <class T>
class blockqueue {
	deque<T> log;
	unsigned int capacity;
	mutex log_mutex;
	condition_variable no_empty;//����
	condition_variable no_full;//���
	bool is_close;
public:
	blockqueue(unsigned int maxsize)
	{
		capacity = maxsize;
		is_close = false;
	}
	void close()
	{
		unique_lock<mutex>lock(log_mutex);
		is_close = true;
		no_empty.notify_all();
	}
	~blockqueue()
	{
		close();
	}
	void push(const T&logtask);
	bool pop(T&logtask);
};
class LOG {
	blockqueue<string>* block_queue;
	thread write;
	mutex log_mutex;
	fstream file;
	char* buffer_;
	void writefunction();
	LOG()
	{
		blockqueue<string>* bq;
		bq = new blockqueue<string>(20);
		block_queue = bq;
		write=thread(&LOG::writefunction,this);
	}
	~LOG()
	{
		if (block_queue) {
			block_queue->close();
		}
		write.join();
		delete block_queue;
	}
public:
	LOG(LOG&) = delete;
	LOG& operator=(const LOG&) = delete;
	static LOG* instance();
	void write_log(SOCKET client,string username,string path);
	void write_log(SOCKET client, string path);
};
 
template <class T>
void blockqueue<T>::push(const T& logtask)
{
	unique_lock<mutex> lock(log_mutex);
	if (log.size() >= capacity)
	{
		no_full.wait(lock, [this]() {return log.size() < capacity; });
	}
	log.push_back(logtask);
	lock.unlock();
	no_empty.notify_one();
}
template <class T>
bool blockqueue<T>::pop(T& logtask)
{
	unique_lock<mutex>lock(log_mutex);
	no_empty.wait(lock, [this]() {return !log.empty()||is_close; });
	if (log.empty())
	{
		return false;
	}
	logtask = log.front();
	log.pop_front();
	lock.unlock();
	no_full.notify_one();
	return true;
}
