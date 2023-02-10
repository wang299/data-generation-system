// 本程序演示用信号量量实现高速缓存。
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <vector>
#include <semaphore.h>

using namespace std;


struct st_message// 缓存队列消息的结构体。
{
	int  msgid;     // 消息的id。
  char message[1024];   // 消息的内容。
}stmsg;

sem_t lock;
sem_t notify;

vector<struct st_message>vmsg;

void  incache(int sig);      // 生产者、数据入队。
void *outcache(void *arg);   // 消费者、数据出队线程的主函数。

int main()
{
  signal(15,incache);  // 接收15的信号，调用生产者函数。

	sem_init(&lock,0,1);
	sem_init(&notify,0,0);
		
  // 创建三个消费者线程。
	pthread_t thid1,thid2,thid3;
	pthread_create(&thid1,NULL,outcache,NULL);
	pthread_create(&thid2,NULL,outcache,NULL);
	pthread_create(&thid3,NULL,outcache,NULL);


	pthread_join(thid1,NULL);
	pthread_join(thid2,NULL);
	pthread_join(thid3,NULL);

	sem_destroy(&lock);
	sem_destroy(&notify);

  return 0;
}

void incache(int sig)       // 生产者、数据入队。
{
	static int i = 1;
	
	struct st_message msg;
	memset(&msg,0,sizeof(struct st_message));
	
	sem_wait(&lock);

	msg.msgid = i++; vmsg.push_back(msg);
	msg.msgid = i++; vmsg.push_back(msg);
	msg.msgid = i++; vmsg.push_back(msg);
	
	sem_post(&lock);
	
	sem_post(&notify);
	sem_post(&notify);
	sem_post(&notify);


}


void *outcache(void *arg)    // 消费者、数据出队线程的主函数。
{
	struct st_message msg;

	while(true)
	{	
		sem_wait(&lock);

		while(vmsg.size()==0)
		{
			sem_post(&lock);
			sem_wait(&notify);
			sem_wait(&lock);
		}
		
		memcpy(&msg,&vmsg[0],sizeof(struct st_message));
		vmsg.erase(vmsg.begin());

		sem_post(&lock);

    // 以下是处理业务的代码。
    printf("phid=%ld,mesgid=%d\n",pthread_self(),msg.msgid);
    usleep(100);
  }

}

