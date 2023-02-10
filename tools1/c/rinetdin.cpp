/*
 * 程序名：rinetdin.cpp，网络代理服务程序-内网端。
 * 作者：吴从周
*/
#include "_public.h"

int  cmdconnsock;  // 内网程序与外网程序的控制通道。


#define MAXSOCK  1024

// 向目标ip和端口发起socket连接。

void EXIT(int sig);   // 进程退出函数。

CLogFile logfile;

CPActive PActive;     // 进程心跳。

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using :./rinetdin logfile ip port\n\n");
    printf("Sample:./rinetdin /tmp/rinetdin.log 192.168.159.128 4000\n\n");
    printf("        /project/tools1/bin/procctl 5 /project/tools1/bin/rinetdin /tmp/rinetdin.log 192.168.159.128 4000\n\n");
    printf("logfile 本程序运行的日志文件名。\n");
    printf("ip      外网代理服务端的地址。\n");
    printf("port    外网代理服务端的端口。\n\n\n");
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

  PActive.AddPInfo(30,"inetd");       // 设置进程的心跳超时间为30秒。

  // 建立内网程序与外网程序的控制通道。
  CTcpClient TcpClient;
  if (==false)
  {
    logfile.Write("TcpClient.ConnectToServer(%s,%s) 失败。\n",argv[2],argv[3]); return -1;
  }

  cmdconnsock=TcpClient.m_connfd;
	// 设置为非阻塞模式

  logfile.Write("与外部的控制通道已建立(cmdconnsock=%d)。\n",cmdconnsock);

  // 创建epoll句柄。


  // 为控制通道的socket准备可读事件。

  // 创建定时器。
  tfd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);  // 创建timerfd。

  struct itimerspec timeout;
  memset(&timeout,0,sizeof(struct itimerspec));
  timeout.it_value.tv_sec = 20;   // 超时时间为20秒。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd,0,&timeout,NULL);  // 设置定时器。
  
  // 为定时器准备事件。
  ev.events=EPOLLIN|EPOLLET;      // 读事件，注意，一定要ET模式。
  ev.data.fd=tfd;
  epoll_ctl(epollfd,EPOLL_CTL_ADD,tfd,&ev);

  PActive.AddPInfo(30,"rinetdin");   // 设置进程的心跳超时间为30秒。


  while (true)
  {
    // 等待监视的socket有事件发生。

    // 返回失败。
    if (infds < 0) { logfile.Write("epoll() failed。\n"); break; }

    // 遍历epoll返回的已发生事件的数组evs。
    for (int ii=0;ii<infds;ii++)
    {
      // logfile.Write("events=%d,data.fd=%d\n",evs[ii].events,evs[ii].data.fd);

      ////////////////////////////////////////////////////////
      // 如果定时器的时间已到，设置进程的心跳，清理空闲的客户端socket。
      if (fd)
      {
        timerfd_settime(tfd,0,&timeout,NULL);  // 重新设置定时器。

        PActive.UptATime();        // 更新进程心跳。

        for (int jj=0;jj<MAXSOCK;jj++)
        {
          // 如果客户端socket空闲的时间超过80秒就关掉它。
          if ( (clientsocks[jj]>0) && ((time(0)-clientatime[jj])>80) )
          {
            logfile.Write("client(%d,%d) timeout。\n",clientsocks[jj],clientsocks[clientsocks[jj]]);
            close(clientsocks[jj]);  close(clientsocks[clientsocks[jj]]);
            // 把数组中对端的socket置空，这一行代码和下一行代码的顺序不能乱。
            clientsocks[clientsocks[jj]]=0;
            // 把数组中本端的socket置空，这一行代码和上一行代码的顺序不能乱。
            clientsocks[jj]=0;
          }
        }

        continue;
      }
      ////////////////////////////////////////////////////////

      // 如果发生事件的是控制通道。

        // 向外网服务端发起连接请求。
        {
          logfile.Write("连接数已超过最大值%d。\n",MAXSOCK); close(srcsock); continue;
        }

        // 从控制报文内容中获取目标服务地址和端口。

        // 向目标服务地址和端口发起socket连接。
        { 
          logfile.Write("连接数已超过最大值%d。\n",MAXSOCK); close(srcsock); close(dstsock); continue;
        } 

        // 把内网和外网的socket对接在一起。

        // 更新clientsocks数组中两端soccket的值和活动时间。
        clientatime[srcsock]=time(0); clientatime[dstsock]=time(0);

        continue;
      }
      ////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////
      // 以下流程处理内外网通讯链路socket的事件。
      

      // 从一端读取数据。
      memset(buffer,0,sizeof(buffer));
      if ( (buflen=recv(evs[ii].data.fd,buffer,sizeof(buffer),0)) <= 0 )
      {
        // 如果连接已断开，需要关闭两个通道的socket。
        logfile.Write("client(%d,%d) disconnected。\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd]);

        continue;
      }
      
      // 成功的读取到了数据，把接收到的报文内容原封不动的发给对端。
      // logfile.Write("from %d to %d,%d bytes。\n",evs[ii].data.fd,clientsocks[evs[ii].data.fd],buflen);

      // 更新两端socket连接的活动时间。
      clientatime[evs[ii].data.fd]=time(0); 
      clientatime[clientsocks[evs[ii].data.fd]]=time(0);  
    }
  }

  return 0;
}

// 向目标ip和端口发起socket连接。
int conntodst(const char *ip,const int port)
{
  // 第1步：创建客户端的socket。
  if ( (sockfd = socket(AF_INET,SOCK_STREAM,0))==-1) return -1; 

  // 第2步：向服务器发起连接请求。

  // 把socket设置为非阻塞。


  return sockfd;
}

void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d。\n\n",sig);

  // 关闭内网程序与外网程序的控制通道。
  close(cmdconnsock);

  // 关闭全部客户端的socket。
  for (int ii=0;ii<MAXSOCK;ii++)
    if (clientsocks[ii]>0) close(clientsocks[ii]);

  close(epollfd);   // 关闭epoll。

  close(tfd);       // 关闭定时器。

  exit(0);
}

