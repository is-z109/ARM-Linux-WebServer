#pragma once
#include<mutex>
#include<thread>
#include<vector>
#include<queue>
#include<condition_variable>
#include<functional>
#include<future>
using namespace std;
class threadpool {                              //���̳߳���
	vector<thread>workers;
	queue<function<void()>> tasks;
	mutex queue_mutex;
	condition_variable condition;
	bool stop;
public:
	threadpool(unsigned int threadnums)
	{
		stop = false;
		for (int i = 0; i < threadnums; i++)
		{
			workers.emplace_back([this](){
				while (1)
				{
					unique_lock<mutex> lock(this->queue_mutex);
					this->condition.wait(lock, [this]() {return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					function<void()> task = move(this->tasks.front());
					this->tasks.pop();
					lock.unlock();
					task();
				}
				});

		}
		cout << "have" << threadnums << "threads" << endl;
	}
	~threadpool()
	{
		unique_lock<mutex>lock(queue_mutex);
		stop = true;
		lock.unlock();
		condition.notify_all();
		for (int i = 0; i < workers.size(); i++)
		{
			workers[i].join();
		}
	}
	void enqueue(function<void()> task)
	{
		unique_lock<mutex>lock(queue_mutex);
		tasks.push(move(task));
		lock.unlock();
		condition.notify_one();
	}
};
