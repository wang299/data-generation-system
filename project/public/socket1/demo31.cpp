/*
 * 程序名：demo31.cpp，此程序用于演示socket通讯的客户端。
 * 作者：吴从周。
*/
#include"../_public.h"

int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("Using:./demo31 ip port\nExample:./demo07 192.168.159.128 5005\n\n"); return -1;
  }


	CTcpClient TcpClient;

	if(TcpClient.ConnectToServer(argv[1],atoi(argv[2]))==false)
	{
		printf("连接失败\n");	
		return -1;
	}


	CLogFile logfile;
	logfile.Open("/tmp/demo31.log","a+");

	int pid =fork();
  char buffer[102400];
 
  // 第3步：与服务端通讯，发送一个报文后等待回复，然后再发下一个报文。
  for (int ii=0;ii<10000;ii++)
  {
   if(pid>0)
		  {
			SPRINTF(buffer,sizeof(buffer),"这是第%d个超级女生，编号%03d。",ii+1,ii+1);

    	if (TcpClient.Write(buffer)<=0) // 向服务端发送请求报文。
    	{ perror("send"); break; }
   	 	
			logfile.Write("发送：%s\n",buffer);
			
			}
   
	 else
	 	{
		memset(buffer,0,sizeof(buffer));
    if (TcpClient.Read(buffer)==false) // 接收服务端的回应报文。
    {
      break;
    }
    
		logfile.Write("接收：%s\n",buffer);
		}
  }
 
}

