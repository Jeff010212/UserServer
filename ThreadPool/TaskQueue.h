#pragma once
#include<queue>
#include<pthread.h>
using callback=void (*)(void* arg);

struct Task{
	Task(){
		function=nullptr;
		arg=nullptr;
	}
	Task(callback f,void* arg){
		this->arg=arg;
		function=f;
	}
	callback function;
	void* arg;
};


class TaskQueue{
public:
	TaskQueue();
	~TaskQueue();

	void addTask(Task task);
	void addTask(callback f,void* arg);
	Task takeTask();

	inline size_t taskNumber(){
		return m_taskQ.size();
	}
private:
	pthread_mutex_t m_mutex;
	std::queue<Task> m_taskQ;
};
