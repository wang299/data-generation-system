// 本程序演示线程资源的回收（分离线程）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER; 			// 声明自旋锁

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

	pthread_rwlock_destroy(&rwlock);
}


void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<100;ii++)
  {
		printf("线程%lu开始申请读锁...\n",pthread_self());

		pthread_rwlock_rdlock(&rwlock); // 加锁
    printf("线程%lu申请读锁成功\n\n",pthread_self());
		sleep(5);
  	pthread_rwlock_unlock(&rwlock);
		printf("线程%lu释放锁成功\n\n",pthread_self());
		
		if(ii==3) sleep(8);
	}

}

void handle(int sig)
{
	printf("开始申请写锁\n");
	pthread_rwlock_wrlock(&rwlock);
	printf("申请写锁成功\n\n");
	sleep(10);
	pthread_rwlock_unlock(&rwlock);
	printf("写锁已经释放\n\n");


}
