/*
 * 程序名：tcppoll.cpp，此程序用于演示采用poll模型的使用方法。
 * 作者：吴从周
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

#define MAXNFDS 1024
// 初始化服务端的监听端口。
int initserver(int port);

int main(int argc,char *argv[])
{
  if (argc != 2) { printf("usage: ./tcppoll port\n"); return -1; }

  // 初始化服务端用于监听的socket。
  int listensock = initserver(atoi(argv[1]));
  printf("listensock=%d\n",listensock);

  if (listensock < 0) { printf("initserver() failed.\n"); return -1; }

	struct pollfd fds[MAXNFDS];	// fds存放需要监视的socket
	for(int i=0;i<MAXNFDS;i++) fds[i].fd =-1;		// 初始化数组，吧全部fd设置为-1，因为poll会跳过负数的fd
	fds[listensock].fd =listensock;
	fds[listensock].events=POLLIN;
	
  int maxfd=listensock;        // 记录集合中socket的最大值。


  while (true)
  {
    // 事件：1)新客户端的连接请求accept；2)客户端有报文到达recv，可以读；3)客户端连接已断开；
    //       4)可以向客户端发送报文send，可以写。
    // 可读事件  可写事件
    // poll() 等待事件的发生(监视哪些socket发生了事件)。

    int infds=poll(fds,maxfd+1,-1); // &timeout

    // 返回失败。
    if (infds < 0)
    {
      perror("poll() failed"); break;
    }

    // 超时，在本程序中，poll函数最后一个参数为空，不存在超时的情况，但以下代码还是留着。
    if (infds == 0)
    {
      printf("poll() timeout.\n"); continue;
    }

    // 如果infds>0，表示有事件发生的socket的数量。
    for (int eventfd=0;eventfd<=maxfd;eventfd++)
    {	
			if(fds[eventfd].fd<0) continue;

			if((fds[eventfd].revents&POLLIN)==0) continue;  //如果没有事件
			

			fds[eventfd].revents=0; //清空
      // 如果发生事件的是listensock，表示有新的客户端连上来。
      if (eventfd==listensock)
      {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
        if (clientsock < 0) { perror("accept() failed"); continue; }

        printf ("accept client(socket=%d) ok.\n",clientsock);

        // 把新客户端的socket加入可读socket的集合。
				fds[clientsock].fd=clientsock;
				fds[clientsock].events=POLLIN;
				fds[clientsock].revents = 0;
        if (maxfd<clientsock) maxfd=clientsock;    // 更新maxfd的值。
      }
      else
      {
        // 如果是客户端连接的socke有事件，表示有报文发过来或者连接已断开。

        char buffer[1024]; // 存放从客户端读取的数据。
        memset(buffer,0,sizeof(buffer));
        if (recv(eventfd,buffer,sizeof(buffer),0)<=0)
        {
          // 如果客户端的连接已断开。
          printf("client(eventfd=%d) disconnected.\n",eventfd);
          close(eventfd);            // 关闭客户端的socket
          fds[eventfd].fd =-1;

          // 重新计算maxfd的值，注意，只有当eventfd==maxfd时才需要计算。
          if (eventfd == maxfd)
          {
            for (int ii=maxfd;ii>0;ii--)  // 从后面往前找。
            {
              if (fds[ii].fd!=-1)
              {
                maxfd = ii; break;
              }
            }
          }
        }
        else
        {
          // 如果客户端有报文发过来。
          printf("recv(eventfd=%d):%s\n",eventfd,buffer);
          // 把接收到的报文内容原封不动的发回去。
          send(eventfd,buffer,strlen(buffer),0);
        }
      }
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


