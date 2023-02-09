/*
 * 程序名：inetd.cpp，网络代理。
 * 作者：吴从周
*/

#include "_public.h"

// 代理路由参数的结构体
struct st_route
{
	int listenport;		// 本地监听的通信端口
	char dstip[31];		// 目标主机的ip地址
	int dstport;			// 目标主机的通信端口
	int listensock;		// 本地监听的sock
}stroute;

void EXIT(int sig);
CLogFile logfile;
CPActive PActive; // 进程心跳

int epollfd=0;// epoll句柄
int tfd;		// 定时器句柄

vector<struct st_route> vroute;// 代理路由的容器
bool loadroute(const char * inifile);// 加载的函数
#define MAXSOCK 1024

// 向目标ip和端口发起sock连接
int conntodst(const char *ip,const int port);

// 初始化服务端的监听端口。
int initserver(int port);

int clientsocks[MAXSOCK];   // 每个socket连接对端的sock的值
int clientatime[MAXSOCK];   // 每个socket连接最后一次收发报文的时间

int main(int argc,char *argv[])
{
  if (argc != 3)
	{ 
		printf("\n");
		printf("using: ./inetd logfile inifile\n\n");
		printf("Sample:./inetd /tmp/inetd.log /project/tools1/ini/inetd.conf\n\n");
		printf("				/project/tools1/bin/procctl 5 /project/tools1/bin/inetd /tmp/inetd.log /project/tools1/ini/inetd.conf\n\n");
		return -1; 
	}

	// 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程。
  // 但请不要用 "kill -9 +进程号" 强行终止。
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // 打开日志文件。
  if (logfile.Open(argv[1],"a+")==false)
  {
    printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
  }
	
	PActive.AddPInfo(30,"inetd");

	// 把代理路由的参数加载到容器
	if(loadroute(argv[2])==false) return -1;
	
	logfile.Write("加载代理路由参数成功(%d)\n",vroute.size());
	
	for(int i=0;i<vroute.size();i++)
	{
		if((vroute[i].listensock=initserver(vroute[i].listenport))<0)
		{
			logfile.Write("initserver(%d)failed\n",vroute[i].listenport);
			return -1;
		}
		fcntl(vroute[i].listensock,F_SETFL,fcntl(vroute[i].listensock,F_GETFD,0)|O_NONBLOCK);
	}

	// 创建epoll句柄
	epollfd=	epoll_create(1);
	struct epoll_event ev;   		// 声明事件的结构体

	// 为监听事件准备可读事件
	for(int i=0;i<vroute.size();i++)
	{
		ev.events = EPOLLIN;
		ev.data.fd = vroute[i].listensock;		// 指定事件的自定义数据，会随着epoll_wait()返回的事件一起返回
		epoll_ctl(epollfd,EPOLL_CTL_ADD,vroute[i].listensock,&ev);  // 吧监听的事件加入epollfd中
	}

	// 创建定时器
  tfd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);  // 创建timerfd。

	struct itimerspec timeout;
	memset(&timeout,0,sizeof(struct itimerspec));
	timeout.it_value.tv_sec=20;	// 超时时间为20s
	timeout.it_value.tv_nsec=0;
	timerfd_settime(tfd,0,&timeout,NULL);
	
	// 为定时器准备事件
	ev.events=EPOLLIN|EPOLLET;  // 读事件，注意，一定要et模式
	ev.data.fd=tfd;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,tfd,&ev);


	struct epoll_event evs[10];


  while (true)
  {

		// 等待监视的事件发生
		int infds = epoll_wait(epollfd,evs,10,-1);
		
    // 返回失败。
    if (infds < 0)
    {
      perror("select() failed"); break;
    }
 	
    // 如果infds>0，表示有事件发生的socket的数量。
    for (int i=0;i<infds;i++)
    {
			
			logfile.Write("events=%d,data,data.fd=%d\n",evs[i].events,evs[i].data.fd);
			//////////////////////////////////////////////////////////////////	
			// 如果定时器的时间已到，设置进程的心跳，清理空闲的客户端socket
			if(evs[i].data.fd==tfd)
			{
				timerfd_settime(tfd,0,&timeout,NULL);// 重新设置
				
				PActive.UptATime();  // 更新心跳
				for(int j=0;j<MAXSOCK;j++)
				{
					// 把空闲时间超过80的客户端关闭
					if((clientsocks[j]>0)&&(time(0)-clientatime[j]>80))
					{
						logfile.Write("client(%d,%d) timeout\n",clientsocks[j],clientsocks[clientsocks[j]]);
						close(clientsocks[j]); close(clientsocks[clientsocks[j]]);
						clientsocks[clientsocks[j]]=0;
						clientsocks[j]=0;
					
					}
				}

				continue;
			}
			
			//////////////////////////////////////////////////////////////////
			int j=0;
			for(j=0;j<vroute.size();j++)
			{	
				///////////////////////
		
				/////////////////////////////////////////////////////////////////////////
      	// 如果发生事件的是listensock，表示有新的客户端连上来。
      	if (evs[i].data.fd==vroute[j].listensock)
      	{	
					// 接受客户端连接
        	struct sockaddr_in client;
        	socklen_t len = sizeof(client);
        	int srcsock = accept(vroute[j].listensock,(struct sockaddr*)&client,&len);
        	if (srcsock < 0) break;
					if (srcsock > MAXSOCK)
					{
						logfile.Write("连接超过最大值\n");close(srcsock);break;
					}

        	printf ("accept client(socket=%d) ok.\n",srcsock);

        	//向目标ip和端口发起连接
					int dstsock = conntodst(vroute[j].dstip,vroute[j].dstport);
					if (dstsock < 0) break;
					if (dstsock > MAXSOCK)
					{
						logfile.Write("连接超过最大值\n");close(dstsock);break;
					}
					 logfile.Write("accept on port %d client(%d,%d) ok\n",vroute[j].listensock,srcsock,dstsock);

					// 为新连接的2个sock准备可读事件，并添加到epoll中。
       		ev.data.fd=srcsock;
					ev.events=EPOLLIN;
					epoll_ctl(epollfd,EPOLL_CTL_ADD,srcsock,&ev);

					ev.data.fd=dstsock;
					ev.events=EPOLLIN;
					epoll_ctl(epollfd,EPOLL_CTL_ADD,dstsock,&ev);
					
					// 更新clientsocks数组中两端socket的值和活动时间
					clientsocks[srcsock] = dstsock; clientsocks[dstsock] = srcsock;
					clientatime[dstsock] = time(0); clientatime[srcsock] = time(0);
      		
					break;
				}
			}

			if(j<vroute.size()) continue; 
      
			//////////////////////////////////////////////////////
			

			//////////////////////////////////////////////////////
      // 如果是客户端连接的socke有事件，表示有报文发过来或者连接已断开。
				
      char buffer[5000]; // 存放从客户端读取的数据。
			int buflen=0;			 // 从socket中读取到的数据的大小

			memset(buffer,0,sizeof(buffer));
      if ((buflen=recv(evs[i].data.fd,buffer,sizeof(buffer),0))<=0)
      {
        // 如果客户端的连接已断开，需要端口2个通道的socket
        logfile.Write("client(%d,%d) disconnected.\n",evs[i].data.fd,clientsocks[evs[i].data.fd]);
       	close(evs[i].data.fd);              // 关闭客户端的socket
				close(clientsocks[evs[i].data.fd]); // 关闭客户端对端的socket
				clientsocks[clientsocks[evs[i].data.fd]]=0;
				clientsocks[evs[i].data.fd]=0;
				continue;
      }

			// 如果客户端有报文发过来。
		//	logfile.Write("from %d to %d %d bytes\n",evs[i].data.fd,clientsocks[evs[i].data.fd],buflen);
 	   /*UpdateStr(buffer,"192.168.159.128:3081","www.weather.com.cn:80");
		 buflen = strlen(buffer);*/

		 // 把接收到的报文内容原封不动的发回去。
      send(clientsocks[evs[i].data.fd],buffer,buflen,0);
			clientatime[evs[i].data.fd] = time(0);
			clientatime[clientsocks[evs[i].data.fd]]=time(0);

			
    }
		
  }


  return 0;
}

// 初始化服务端的监听端口。
int initserver(int port)
{
  int sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock < 0)
  {
    perror("socket() failed"); return -1;
  }

  int opt = 1; unsigned int len = sizeof(opt);
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
  {
    perror("bind() failed"); close(sock); return -1;
  }

  if (listen(sock,5) != 0 )
  {
    perror("listen() failed"); close(sock); return -1;
  }

  return sock;
}

int conntodst(const char *ip,const int port)
{
	int sockfd;
  if ( (sockfd = socket(AF_INET,SOCK_STREAM,0))==-1) return -1;

  // 第2步：向服务器发起连接请求。
  struct hostent* h;
  if ( (h = gethostbyname(ip)) == 0 )   { close(sockfd);return -1;}

  struct sockaddr_in servaddr;
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port); // 指定服务端的通讯端口。
  memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);
  
	fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK);
	
	connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
  
	return sockfd;
}

void EXIT(int sig)
{
	logfile.Write("程序退出，sig=%d\n",sig);
	
	// 关闭全部监听的sock
	for(int i=0;i<vroute.size();i++)
		close(vroute[i].listensock);
	// 关闭全部客户端sock
	for(int i=0;i<MAXSOCK;i++)
		if(clientsocks[i]>0)
			close(clientsocks[i]);
	
	close(epollfd);
	close(tfd);
	exit(0);

}

bool loadroute(const char * inifile)
{
	CFile File;
	if(File.Open(inifile,"r")==false)
	{
		logfile.Write("打开参数文件失败\n");
	}

	char strBuffer[256];
	CCmdStr CmdStr;
	while(true)
	{
		memset(strBuffer,0,sizeof(strBuffer));

		if(File.FFGETS(strBuffer,200)==false) break;
		char *pos = strstr(strBuffer,"#");
		if(pos!=0) pos[0]=0;							// 删除说明文字
		DeleteRChar(strBuffer,' ');				// 删除右边的空格
		UpdateStr(strBuffer,"  "," ",true); // 2个空格变成一个
		CmdStr.SplitToCmd(strBuffer," ");	
		if(CmdStr.CmdCount()!=3) continue;

		memset(&stroute,0,sizeof(struct st_route));
		CmdStr.GetValue(0,&stroute.listenport);
		CmdStr.GetValue(1, stroute.dstip);
		CmdStr.GetValue(2,&stroute.dstport);

		vroute.push_back(stroute);
	}
	
	return true;
}
