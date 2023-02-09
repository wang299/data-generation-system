#include "_public.h"

CLogFile logfile;
int main(int argc,char *argv[])
{
	if(argc!=4)
 	{	
		printf("重新输入参数\n");
		return -1;
  }

	if (logfile.Open(argv[3])==false)
	{
		printf("logfile.Open(%s) failed.\n",argv[3]);
		return -1;
	}
	
	for(int i=0;i<argc;i++)
	{
		printf(argv[i]);
		printf("\n");
	}
	logfile.Write("crtsurfdata 开始运行。\n");
	logfile.Write("crtsurfdata 运行结束。\n");

	//这里插入业务代码
	return 0;
}
