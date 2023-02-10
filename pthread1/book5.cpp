// 本程序演示线程资源的回收（分离线程）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int var;
pthread_spinlock_t spin; 			// 声明自旋锁

void *thmain(void *arg);    // 线程主函数。

int main(int argc,char *argv[])
{
	// 初始化锁
	pthread_spin_init(&spin,PTHREAD_PROCESS_PRIVATE);

  pthread_t thid1,thid2;
  // 创建线程。
  if (pthread_create(&thid1,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
	
  if (pthread_create(&thid2,NULL,thmain,NULL)!=0) { printf("pthread_create failed.\n"); exit(-1); }
	

  printf("join...\n");
 	pthread_join(thid1,NULL);
	pthread_join(thid2,NULL);
  printf("join ok.\n");
	
	printf("var =%d\n ",var);

	pthread_spin_destroy(&spin);
}


void *thmain(void *arg)    // 线程主函数。
{
  for (int ii=0;ii<100000;ii++)
  {
		pthread_spin_lock(&spin);
    var++;
  	pthread_spin_unlock(&spin);
	}

}
