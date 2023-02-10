/*
 * 程序名：demo32.cpp，此程序用于演示socket通讯的服务端。
 * 作者：吴从周
*/
 
#include"../_public.h"


int main(int argc,char *argv[])
{
  if (argc!=2)
  {
    printf("Using:./demo32 port\nExample:./demo32 5005\n\n"); return -1;
  }

	CLogFile logfile;
	CTcpServer TcpServer;
	
	logfile.Open("/tmp/demo32.log","a+");

	if(TcpServer.InitServer(atoi(argv[1])) ==false)
	{
		printf("服务端初始化失败\n");
		return -1;
	}

	if(TcpServer.Accept() ==false)
	{
		printf("accept failed\n");
	}

	printf("客户端%s已经连接\n",TcpServer.GetIP());

  char buffer[102400];

  // 第5步：与客户端通讯，接收客户端发过来的报文后，回复ok。
  while (1)
  {
    memset(buffer,0,sizeof(buffer));
    if(TcpServer.Read(buffer)==false) break;
    logfile.Write("接收：%s\n",buffer);

    strcpy(buffer,"ok");
    if (TcpServer.Write(buffer)==false) break; // 向客户端发送响应结果。
    logfile.Write("发送：%s\n",buffer);
  }
 
}
