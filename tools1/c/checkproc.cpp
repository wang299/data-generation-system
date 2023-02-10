#include"_public.h"


//程序运行的日志
CLogFile logfile;

int main(int argc,char *argv[])
{
	//帮助
	if(argc !=2)
	{
		printf("\n");
		printf("Using: ./checkproc logfilename \n");
		printf("Example: /project/tools1/bin/procctl 10 /project/tools1/bin/checkproc /tmp/log/checkproc.log\n\n");

		printf("此程序用于检查后台程序是否超时，如果超时，则重启\n");
		return 0;
	}
	
	//打开日志文件
	if(logfile.Open(argv[1],"a+") ==false)
	{
		printf("logfile failed\n");
	}
	
	int shmid=0;
	if((shmid = shmget((key_t)SHMKEYP,MAXNUMP*sizeof(struct st_procinfo),0666|IPC_CREAT))==-1)
	{
		logfile.Write("创建共享内存失败\n");
		return false;
			
	}

	//连接
	struct st_procinfo *shm = (struct st_procinfo *)shmat(shmid,0,0);

	//遍历
	for(int i= 0;i<MAXNUMP;i++)
	{	
		//pid =0
		//pid!=0
	 // 如果记录的pid==0，表示空记录，continue;
    if (shm[i].pid==0) continue;

    // 如果记录的pid!=0，表示是服务程序的心跳记录。

    // 程序稳定运行后，以下两行代码可以注释掉。
    logfile.Write("ii=%d,pid=%d,pname=%s,timeout=%d,atime=%d\n",\
                   i,shm[i].pid,shm[i].pname,shm[i].timeout,shm[i].atime);
	
		int iret = kill(shm[i].pid,0);
		if(iret==-1)
		{
			logfile.Write("进程pid=%d(%s)已经不存在。\n",(shm+i)->pid,(shm+i)->pname);
			memset(shm+i,0,sizeof(struct st_procinfo));
			continue;
		}
		
		time_t now = time(0);
		
		if(now-shm[i].atime<shm[i].timeout) continue;
		
		logfile.Write("进程pid = %d已超时\n",(shm+i)->pid);

		kill(shm[i].pid,15);

		for(int j=0;j<5;j++)
		{
			sleep(1);
			iret = kill(shm[i].pid,0);
			if(iret ==-1);
				break;
		}
		if(iret ==-1)
		{
			logfile.Write("进程pid=%d已正常终止\n",shm[i].pid);
		}

		else
		{
			kill(shm[i].pid,9);	
			logfile.Write("进程pid=%d已强行终止\n",shm[i].pid);
		}

		memset(shm+i,0,sizeof(struct st_procinfo));
	}
	return 0;
}
