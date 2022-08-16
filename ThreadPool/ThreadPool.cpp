#include"ThreadPool.h"
#include<iostream>
#include<string.h>
#include<string>
#include<unistd.h>
using namespace std;

ThreadPool::ThreadPool(int min,int max){
	do{
		taskQ=new TaskQueue;
		if(taskQ==nullptr){
			cout<<"malloc taskQ fail...\n";
			break;
		}

		threadIDs=new pthread_t[max];
		if(threadIDs==nullptr){
			cout<<"malloc threadIDs fail...\n";
			break;
		}
		memset(threadIDs,0,sizeof(pthread_t)*max);
		minNum=min;
		maxNum=max;
		busyNum=0;
		liveNum=min;
		exitNum=0;

		if(pthread_mutex_init(&mutexPool,NULL)!=0||pthread_cond_init(&notEmpty,NULL)!=0){
			cout<<"mutex or condition init fail...\n";
			break;
		}

		shutdown=false;

		pthread_create(&managerID,NULL,manager,this);
		for(int i=0;i<min;i++)pthread_create(&threadIDs[i],NULL,worker,this);
		return;
	}while(0);

	if(threadIDs) delete[]threadIDs;
	if(taskQ) delete taskQ;
}


ThreadPool::~ThreadPool(){
	shutdown=true;
	pthread_join(managerID,NULL);
	for(int i=0;i<liveNum;i++)pthread_cond_signal(&notEmpty);

	if(taskQ)delete taskQ;

	if(threadIDs) delete[]threadIDs;

	pthread_mutex_destroy(&mutexPool);
	pthread_cond_destroy(&notEmpty);
}



void ThreadPool::addTask(Task task){
	if(shutdown)return;
	taskQ->addTask(task);

	pthread_cond_signal(&notEmpty);

}



int ThreadPool::getBusyNum(){
	pthread_mutex_lock(&mutexPool);
	int busyNum=this->busyNum;
	pthread_mutex_unlock(&mutexPool);
	return busyNum;

}



int ThreadPool::getAliveNum(){
	pthread_mutex_lock(&mutexPool);
	int liveNum=this->liveNum;
	pthread_mutex_unlock(&mutexPool);
	return liveNum;
}



void* ThreadPool::worker(void* arg){
	ThreadPool* pool=static_cast<ThreadPool*>(arg);

	while(1){
		pthread_mutex_lock(&pool->mutexPool);

		while(!pool->shutdown && pool->taskQ->taskNumber()<=0){
			pthread_cond_wait(&pool->notEmpty,&pool->mutexPool);

			if(pool->exitNum>0){
				pool->exitNum--;
				if(pool->liveNum>pool->minNum){
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					pool->threadExit();
				}
			}
		}

		if(pool->shutdown){
			pthread_mutex_unlock(&pool->mutexPool);
			pool->threadExit();
		}

		Task task=pool->taskQ->takeTask();

		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexPool);
		cout<<"thread "<<to_string(pthread_self())<<" start working...\n";

		task.function(task.arg);
		delete task.arg;
		task.arg=nullptr;

		cout<<"thread "<<to_string(pthread_self())<<" end working...\n";

		pthread_mutex_lock(&pool->mutexPool);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexPool);
	}
	return NULL;

}



void* ThreadPool::manager(void* arg){
	ThreadPool* pool=static_cast<ThreadPool*>(arg);
	while(!pool->shutdown){
		sleep(3);

		pthread_mutex_lock(&pool->mutexPool);
		int queueSize=pool->taskQ->taskNumber();
		int liveNum=pool->liveNum;
		int busyNum=pool->busyNum;
		pthread_mutex_unlock(&pool->mutexPool);

		if(queueSize>liveNum && liveNum<pool->maxNum){

			pthread_mutex_lock(&pool->mutexPool);
			int counter=0;
			for(int i=0;i<pool->maxNum && counter<NUMBER && pool->liveNum<pool->maxNum;i++){
				if(pool->threadIDs[i]==0){
					pthread_create(&pool->threadIDs[i],NULL,worker,pool);
					counter++;
					pool->liveNum++;
				}
			}
			pthread_mutex_unlock(&pool->mutexPool);
		}

		if(busyNum*2<liveNum && liveNum>pool->minNum){
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum=NUMBER;
			pthread_mutex_unlock(&pool->mutexPool);
			for(int i=0;i<NUMBER;i++)pthread_cond_signal(&pool->notEmpty);
		}

	}
	return NULL;

}

void ThreadPool::threadExit(){
	pthread_t tid=pthread_self();
	for(int i=0;i<maxNum;i++){
		if(threadIDs[i]==tid){
			threadIDs[i]=0;
			cout<<"threadExit() called,"<<to_string(tid)<<" exiting...\n";
			break;
		}
	}
	pthread_exit(NULL);
}

