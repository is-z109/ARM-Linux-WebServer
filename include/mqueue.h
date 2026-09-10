#pragma once
#include<queue>
#include<functional>
#include<condition_variable>
#include<mutex>
#include<thread>
#include"GPIO.h"
//下位机执行队列
using namespace std;
class mqueue {
	queue<function<void()>> task;
	mutex queue_mutex;
	condition_variable condition;
	thread worker;
	bool stop;
	mqueue()
	{
		stop = false;
		worker=thread([this]() {
			while(1)
			{
			unique_lock<mutex> lock(queue_mutex);
			condition.wait(lock, [this]() {return !task.empty() || stop; });
			if (stop&&task.empty())
			{
				return;
			}
			function<void()> work = move(task.front());
			task.pop();
			lock.unlock();
			work();
			}});

	}
public:
     mqueue(const mqueue&) = delete;
	 mqueue& operator=(const mqueue&) = delete;
	void insert(function<void()> wk)
	{
		unique_lock<mutex>lock(queue_mutex);
		task.push(wk);
		lock.unlock();
		condition.notify_one();
	}
	~mqueue()
	{
		unique_lock<mutex>lock(queue_mutex);
		stop = true;
		lock.unlock();
		condition.notify_all();
		worker.join();
	}
	static mqueue&instance()
	{
		static mqueue lyra;
		return lyra;
	}
};
