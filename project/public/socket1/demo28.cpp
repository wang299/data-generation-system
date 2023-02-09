/*
 * 程序名：demo28.cpp，此程序演示HTTP协议，接收http请求报文。
 * 作者：吴从周
*/
#include "../_public.h"
#include "_ooci.h" 
// 把html文件的内容发送给客户端。
bool SendData(const int sockfd,const char *strget);
bool getvalue(const char * strget,const char *name,char *value,const int len);

int main(int argc,char *argv[])
{
  if (argc!=2)
  {
    printf("Using:./demo28 port\nExample:./demo28 8080\n\n"); return -1;
  }

  CTcpServer TcpServer;

  // 服务端初始化。
  if (TcpServer.InitServer(atoi(argv[1]))==false)
  {
    printf("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  // 等待客户端的连接请求。
  if (TcpServer.Accept()==false)
  {
    printf("TcpServer.Accept() failed.\n"); return -1;
  }

  printf("客户端（%s）已连接。\n",TcpServer.GetIP());

	char strget[10240];
  char strsend[102400];
	memset(strget,0,sizeof(strget));
  // 接收http客户端发送过来的报文。
  recv(TcpServer.m_connfd,strget,1000,0);

  printf("%s\n",strget);

  // 先把响应报文头部发送给客户端。
  memset(strsend,0,sizeof(strsend));
  sprintf(strsend,\
         "HTTP/1.1 200 OK\r\n"\
         "Server: demo28\r\n"\
         "Content-Type: text/html;charset=utf-8\r\n"\
				 "\r\n");
        // "Content-Length: 105316\r\n\r\n");
  if (Writen(TcpServer.m_connfd,strsend,strlen(strsend))== false) return -1;

//  logfile.Write("%s",strsend);

  // 再把html文件的内容发送给客户端。
  SendData(TcpServer.m_connfd,strget);
}

// 把html文件的内容发送给客户端。
bool SendData(const int sockfd,const char *strget)
{
	char username[31],passwd[31],intername[30],obtid[11],begintime[21],endtime[21];
	memset(username,0,sizeof(username));
	memset(passwd,0,sizeof(passwd));
	memset(intername,0,sizeof(intername));
	memset(obtid,0,sizeof(obtid));
	memset(begintime,0,sizeof(begintime));
	memset(endtime,0,sizeof(endtime));

	getvalue(strget,"username",username,30);
	getvalue(strget,"passwd",passwd,30);
	getvalue(strget,"intername",intername,30);
	getvalue(strget,"obtid",obtid,10);
	getvalue(strget,"begintime",begintime,20);
	getvalue(strget,"endtime",endtime,20);
	
	printf("username=%s\n",username);
	printf("passwd=%s\n",passwd);
	printf("intername=%s\n",intername);
	printf("obtid=%s\n",obtid);
	printf("begintime=%s\n",begintime);
	printf("endtime=%s\n",endtime);
	

	connection conn;
	conn.connecttodb("scott/20020914@oracle_wyz","Simplified Chinese_China.AL32UTF8");

	sqlstatement stmt(&conn);
	stmt.prepare("select '<obtid>'||obtid||'</obtid>'||'<ddatetime>'||to_char(ddatetime,'yyyy_mm_dd hh24:miss')||'</ddatetime>'||'<t>'||t||'</t>'||'<p>'||p||'</>'||'<u>'||u||'</u>'||'<keyid>'||keyid||'</keyid>'||'<endl/>' from T_ZHOBTMIND2 where obtid=:1 and ddatetime>to_date(:2,'yyyymmddhh24miss') and ddatetime<to_date(:3,'yyyymmddhh24miss')");
	
	char strxml[1001];
	stmt.bindout(1,strxml,1000);
	stmt.bindin(1,obtid,10);
	stmt.bindin(2,begintime,14);
	stmt.bindin(3,endtime,14);

	stmt.execute();
		
	Writen(sockfd,"<data>\n",strlen("<data>\n"));

	while(true)
	{
		memset(strxml,0,sizeof(strxml));
		if(stmt.next()!=0) break;

		strcat(strxml,"\n");

		Writen(sockfd,strxml,strlen(strxml));

	}
	
	Writen(sockfd,"</data>\n",strlen("</data>\n"));
  return true;
}


bool getvalue(const char * strget,const char *name,char *value,const int len)
{
	value[0]=0;
	char *start,*end;
	start = end = 0;

	start=strstr((char*)strget,(char *)name);
	if(start==0) return false;

	end =strstr(start,"&");
	if(end ==0) end =strstr(start," ");

	if(end ==0) return false;

	int ilen =end-(start+strlen(name)+1);
	if(ilen>len) ilen =len;
	strncpy(value,start+strlen(name)+1,ilen);

	value[ilen] =0;

	return true;
}
