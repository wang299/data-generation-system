// 本程序演示线程资源的回收（分离线程）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int var;
pthread_mutex_t mutex; 			// 声明互斥锁

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
	// 初始化锁
	pthread_mutex_init(&mutex,NULL);

  pthread_t thid1,thid2;
  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
	
  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
	

  printf("join...\n");
 	pthread_join(thid1,NULL);
	pthread_join(thid2,NULL);
  printf("join ok.\n");
	
	printf("var =%d\n ",var);

	pthread_mutex_destroy(&mutex);
}


void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<100;ii++)
  {
		pthread_mutex_lock(&mutex);
    var++;
  	pthread_mutex_unlock(&mutex);
	}

}
