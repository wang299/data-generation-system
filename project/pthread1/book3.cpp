// 本程序演示线程参数的传递（用强制转换的方法传变量的值）。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct phread{
	int a;
	int b;
};

void *thmain1(void *arg);    // 线程1的主函数。

int var;
int main(int argc,char *argv[])
{
  pthread_t thid1=0;   // 线程id typedef unsigned long pthread_t
	struct phread *p = new struct phread;
	p->a =1;p->b =2;
  // 创建线程。
  var=1;
  if (pthread_create(&thid1,NULL,thmain1,p)!=0) { printf("pthread_create failed.\n"); exit(-1); }


  // 等待子线程退出。
  printf("join...\n");
  pthread_join(thid1,NULL);  
  printf("join ok.\n");
}

void *thmain1(void *arg)    // 线程主函数。
{
	struct phread *pst =  (struct phread *)arg;
  printf("var1.a=%d,var1.b=%d\n",pst->a,pst->b);
	delete pst;
  printf("线程1开始运行。\n");
}

