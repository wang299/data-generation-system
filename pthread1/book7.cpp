// 本程序演示线程资源的回收（分离线程）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 	 // 声明条件变量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 声明互斥锁

void *thmain(void *arg);    // 线程主函数。

void handle(int sig);

int main(int argc,char *argv[])
{
	signal(15,handle);
  pthread_t thid1,thid2,thid3;

  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
	sleep(1);

  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
	sleep(1);

  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
	sleep(1);


 	pthread_join(thid1,NULL);
	pthread_join(thid2,NULL);
	pthread_join(thid3,NULL);

	pthread_cond_destroy(&cond);
}


void *thmain(void *arg)    // 线程主函数。
{
	while(true)
	{
		printf("线程%lu开始等待条件信号...\n",pthread_self());
		pthread_cond_wait(&cond,&mutex);
		printf("线程%lu等待条件信号成功\n\n",pthread_self());

	}
}

void handle(int sig)
{
	printf("发送条件信号...\n");
	pthread_cond_signal(&cond);

}
