/*
 * 程序名：webserver.cpp，此程序演示采用开发框架的CTcpServer类实现socket通讯多线程的服务端。
 * 作者：吴从周
*/
#include "_public.h"
#include "_ooci.h"
CLogFile   logfile;    // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

void EXIT(int sig);    // 进程的退出函数。

pthread_spinlock_t vthidlock;  // 用于锁定vthid的自旋锁。
vector<pthread_t> vthid;       // 存放全部线程id的容器。
void *thmain(void *arg);       // 线程主函数。

void thcleanup(void *arg);     // 线程清理函数。
 
// 读取客户端的报文。
int ReadT(const int sockfd,char *buffer,const int size,const int itimeout);

struct st_arg
{ 
  char connstr[101];  // 数据库的连接参数。
  char charset[51];   // 数据库的字符集。
  int  port;          // web服务监听的端口。
} starg;

// 显示程序的帮助
void _help(char *argv[]);

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer);

// 从GET请求中获取参数。
bool getvalue(const char *buffer,const char *name,char *value,const int len);
// 判断用户名和密码。
bool Login(connection *conn,const char *buffer,const int sockfd);
// 判断用户是否有调用接口的权限。
bool CheckPerm(connection *conn,const char *buffer,const int sockfd);
// 执行接口的SQL语句，把数据返回给客户端。
bool ExecSQL(connection *conn,const char *buffer,const int sockfd);

void *checkpool(void *arg);				// 检查线程函数

// 数据库连接池类
class connpool
{
private:
	struct st_conn
	{
		connection conn;				
		pthread_mutex_t mutex;			
		time_t atime;						// 数据库连接上次使用的时间，未连接取0
	}*m_conn;									// 数据库连接池

	int m_maxconns;           // 数据库连接池的最大值
	int m_timeout;						// 超时时间
	char m_connstr[101];			// 连接参数
	char m_charset[101];			// 字符集

public:
	connpool();
	~connpool();
	
	// 初始化数据库连接池
	bool init(char * connstr, char * charset,int maxconns,int timeout);
	// 断开数据库连接
	void destroy();

	// 从数据库连接池中获取一个空闲的连接，成功返回数据库连接的地址
	// 如果连接池已经用完或连接数据库失败，返回空
	connection * get();
	// 归还数据库连接
	bool free(connection *conn);
	
	// 检查数据库连接池，断开空闲的连接，在服务程序中，用一个专用的子线程调用这个函数
	void checkpool();


	
};

connpool oraconnpool;
int main(int argc,char *argv[])
{
  if (argc!=3)
  {
		_help(argv); return -1;
  }
	

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
 	 CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false) { printf("logfile.Open(%s) failed.\n",argv[2]); return -1; }

	if(_xmltoarg(argv[2])==false) 
	{
		logfile.Write("XML failed\n");
		EXIT(-1);
	}

///////////////////////////////////////////////////////
	oraconnpool.init(starg.connstr,starg.charset,2,20);
	connection *conn0 = oraconnpool.get();
	oraconnpool.free(conn0);
	connection *conn1 = oraconnpool.get();
	connection *conn2 = oraconnpool.get();
	connection *conn3 = oraconnpool.get();
	oraconnpool.free(conn1);
	sleep(5);
	oraconnpool.free(conn2);
	sleep(18);
	oraconnpool.checkpool();
	sleep(5);
	oraconnpool.checkpool();
	connection *conn4 = oraconnpool.get();
	connection *conn5 = oraconnpool.get();
	connection *conn6 = oraconnpool.get();
	return 0;
////////////////////////////////////////////////


  // 服务端初始化。
  if (TcpServer.InitServer(starg.port)==false)
  {
    logfile.Write("TcpServer.InitServer(%d) failed.\n",starg.port); EXIT(-1);
  }
	
	// 初始化数据库连接池
	if(oraconnpool.init(starg.connstr,starg.charset,10,50)==false)
	{
		logfile.Write("initconns failed\n");
		return -1;
	}
	
	else
	{
		pthread_t thid;
		if (pthread_create(&thid,NULL,checkpool,0)!=0)
		{
				logfile.Write("pthread_create() failed.\n");  return -1;
		}
	}

  pthread_spin_init(&vthidlock,0);
  
	while (true)
  {
    // 等待客户端的连接请求。
    if (TcpServer.Accept()==false)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); EXIT(-1);
    }

    logfile.Write("客户端（%s）已连接。\n",TcpServer.GetIP());

    // 创建一个新的线程，让它与客户端通讯。
    pthread_t thid;
    if (pthread_create(&thid,NULL,thmain,(void *)(long)TcpServer.m_connfd)!=0)
    {
      logfile.Write("pthread_create() failed.\n"); TcpServer.CloseListen(); continue;
    }

    pthread_spin_lock(&vthidlock);
    vthid.push_back(thid);    // 把线程id放入容器。
    pthread_spin_unlock(&vthidlock);
  }
}

void *thmain(void *arg)     // 线程主函数。
{
  pthread_cleanup_push(thcleanup,arg);       // 把线程清理函数入栈（关闭客户端的socket）。

  int connfd=(int)(long)arg;    // 客户端的socket。

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);   // 线程取消方式为立即取消。

  pthread_detach(pthread_self());           // 把线程分离出去。

	
	char strrecvbuf[1024];
  memset(strrecvbuf,0,sizeof(strrecvbuf));
  if(ReadT(connfd,strrecvbuf,sizeof(strrecvbuf),3)<=0)
  {
    logfile.Write("ReadT faild\n");
    pthread_exit(0);
  }

  if(strncmp(strrecvbuf,"GET",3)!=0)
  {
    logfile.Write("Get faild\n");
    pthread_exit(0);
  }

  
	connection *conn=oraconnpool.get(); // 数据库连接。
	char strbuffer[256];
	if(conn==0)
	{
		usleep(100000);
    memset(strbuffer,0,sizeof(strbuffer));

    sprintf(strbuffer,\
    	"HTTP/1.1 200 OK\r\n"\
    	"Server:webserver\r\n"\
    	"Content-Type:text/html;charset=utf-8\r\n\r\n"\
			"<retcode>-1</retcode><message>internal error</message>");
    	
		Writen(connfd,strbuffer,strlen(strbuffer));
		pthread_exit(0);
	}


  // 判断用户名
  if(Login(conn,strrecvbuf,connfd)==false){oraconnpool.free(conn); pthread_exit(0);}

  // 判断用户是否有权限
  if(CheckPerm(conn,strrecvbuf,connfd)==false) {oraconnpool.free(conn);pthread_exit(0);}

	// 把响应报头发送给客户端
    memset(strbuffer,0,sizeof(strbuffer));

    sprintf(strbuffer,\
    "HTTP/1.1 200 OK\r\n"\
    "Server:webserver\r\n"\
    "Content-Type:text/html;charset=utf-8\r\n\r\n");
    Writen(connfd,strbuffer,strlen(strbuffer));

  // 再执行接口的sql语句，把数据返回给客户端 
  if(ExecSQL(conn,strrecvbuf,connfd)==false)  {oraconnpool.free(conn); pthread_exit(0);}	
	
	oraconnpool.free(conn);


  pthread_cleanup_pop(1);         // 把线程清理函数出栈。
}

// 进程的退出函数。
void EXIT(int sig)  
{
  // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  logfile.Write("进程退出，sig=%d。\n",sig);

  TcpServer.CloseListen();    // 关闭监听的socket。

	pthread_spin_lock(&vthidlock);
  // 取消全部的线程。
	// 有时候会跑太快，主程序来不及把线程的id放入容器
	//
  for (int ii=0;ii<vthid.size();ii++)
  {
    pthread_cancel(vthid[ii]);
  }
	
	pthread_spin_unlock(&vthidlock);

  sleep(1);        // 让子线程有足够的时间退出。

  pthread_spin_destroy(&vthidlock);

  exit(0);
}

void thcleanup(void *arg)     // 线程清理函数。
{
  close((int)(long)arg);      // 关闭客户端的socket。

  pthread_spin_lock(&vthidlock);
  for (int ii=0;ii<vthid.size();ii++)
  {
    if (pthread_equal(pthread_self(),vthid[ii])) { vthid.erase(vthid.begin()+ii); break; }
  }

  pthread_spin_unlock(&vthidlock);

  logfile.Write("线程%lu退出。\n",pthread_self());
}


// 显示程序的帮助
void _help(char *argv[])
{
  printf("Using:/project/tools1/bin/webserver logfilename xmlbuffer\n\n");

  printf("Sample:/project/tools1/bin/procctl 10 /project/tools1/bin/webserver /log/idc/webserver.log \"<connstr>scott/20020914@oracle_wyz</connstr><charset>Simplified Chinese_China.AL32UTF8</charset><port>8080</port>\"\n\n");

  printf("本程序是数据总线的服务端程序，为数据中心提供http协议的数据访问接口。\n");
  printf("logfilename 本程序运行的日志文件。\n");
  printf("xmlbuffer   本程序运行的参数，用xml表示，具体如下：\n\n");

  printf("connstr     数据库的连接参数，格式：ip,username,password,dbname,port。\n");
  printf("charset     数据库的字符集，这个参数要与数据源数据库保持一致，否则会出现中文>乱码的情况。\n");
  printf("port        web服务监听的端口。\n");
  printf("timeout     线程等待客户端报文的超时时间，单位：秒，建议取值小于10。\n\n");
}


// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer)
{
  memset(&starg,0,sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer,"connstr",starg.connstr,100);
  if (strlen(starg.connstr)==0) { logfile.Write("connstr is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"charset",starg.charset,50);
  if (strlen(starg.charset)==0) { logfile.Write("charset is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"port",&starg.port);
  if (starg.port==0) { logfile.Write("port is null.\n"); return false; }

  return true;
}


// 读取客户端的报文。
int ReadT(const int sockfd,char *buffer,const int size,const int itimeout)
{
  if (itimeout > 0)
	{
    struct pollfd fds;
		fds.fd = sockfd;
		fds.events = POLLIN;

		int iret;
    if ( (iret = poll(&fds,1,itimeout*1000)) <= 0 ) return iret;
  }

  return recv(sockfd,buffer,size,0);
}

bool Login(connection *conn,const char *buffer,const int sockfd)
{
  char username[31],passwd[31];

  getvalue(buffer,"username",username,30);
  getvalue(buffer,"passwd",passwd,30);

  // 查询表
  sqlstatement stmt(conn);
  stmt.prepare("select count(*) from T_USERINFO where username=:1 and passwd =:2 and rsts=1");
  stmt.bindin(1,username,30);
  stmt.bindin(2,passwd,30);
  int icount=0;
  stmt.bindout(1,&icount);
  stmt.execute();
  stmt.next();


 // 认证失败
  if(icount==0)
  {
    char strbuffer[256];
    memset(strbuffer,0,sizeof(strbuffer));

    sprintf(strbuffer,\
    "HTTP/1.1 200 OK\r\n"\
    "Server:webserver\r\n"\
    "Content-Type:text/html;charset=utf-8\r\n\r\n"\
    "<retcode>-1</retcode><message>username or passwd is invalid</message>");
    Writen(sockfd,strbuffer,strlen(strbuffer));

    return false;
  }
  return true;
}


// 从GET请求中获取参数。
bool getvalue(const char *buffer,const char *name,char *value,const int len)
{
  value[0]=0;

  char *start,*end;
  start=end=0;

  start=strstr((char *)buffer,(char *)name);
  if (start==0) return false;

  end=strstr(start,"&");
  if (end==0) end=strstr(start," ");

  if (end==0) return false;

  int ilen=end-(start+strlen(name)+1);
  if (ilen>len) ilen=len;

  strncpy(value,start+strlen(name)+1,ilen);

  value[ilen]=0;

  return true;
}


bool CheckPerm(connection *conn,const char *buffer,const int sockfd)
{
   char username[31],intername[31];

  getvalue(buffer,"username",username,30);
  getvalue(buffer,"intername",intername,30);

  // 查询表

  sqlstatement stmt(conn);
  stmt.prepare("select count(*) from T_USERANDINTER where username=:1 and intername=:2 and intername in(select intername from T_INTERCFG where rsts=1)");
  stmt.bindin(1,username,30);
  stmt.bindin(2,intername,30);
  int icount=0;
  stmt.bindout(1,&icount);
  stmt.execute();
  stmt.next();
	logfile.Write("count = %d,username = %s ,intername=%s",icount,username,intername);

  // 认证失败
  if(icount!=1)
  {
    char strbuffer[256];
    memset(strbuffer,0,sizeof(strbuffer));

    sprintf(strbuffer,\
    "HTTP/1.1 200 OK\r\n"\
    "Server:webserver\r\n"\
    "Content-Type:text/html;charset=utf-8\r\n\r\n"\
    "<retcode>-1</retcode><message>permission denied</message>");
    Writen(sockfd,strbuffer,strlen(strbuffer));
		
		logfile.Write("失败\n");
    return false;
  }
	logfile.Write("成功\n");
  return true;
}

// 执行接口的SQL语句，把数据返回给客户端。
bool ExecSQL(connection *conn,const char *buffer,const int sockfd)
{
  char username[31],intername[30],selectsql[1001],colstr[301],bindin[301];
  memset(username,0,sizeof(username));
  memset(intername,0,sizeof(intername));
  memset(selectsql,0,sizeof(selectsql)); // 接口SQL。
  memset(colstr,0,sizeof(colstr));       // 输出列名。
  memset(bindin,0,sizeof(bindin));       // 接口参数。

  getvalue(buffer,"username",username,30);    // 获取用户名。
  getvalue(buffer,"intername",intername,30);  // 获取接口名。

  // 把接口的参数取出来。
  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select selectsql,colstr,bindin from T_INTERCFG where intername=:1");
  stmt.bindin(1,intername,30);     // 接口名。
  stmt.bindout(1,selectsql,1000);  // 接口SQL。
  stmt.bindout(2,colstr,300);      // 输出列名。
  stmt.bindout(3,bindin,300);      // 接口参数。
  stmt.execute();  // 这里基本上不用判断返回值，出错的可能几乎没有。
  stmt.next();

  // prepare接口的SQL语句。
  stmt.prepare(selectsql);
	logfile.Write("selectsql = %s\n\n",selectsql);
  CCmdStr CmdStr;

  ////////////////////////////////////////
  // 拆分输入参数bindin。
  CmdStr.SplitToCmd(bindin,",");
	logfile.Write("bindin = %s\n\n",bindin);
  // 用于存放输入参数的数组，输入参数的值不会太长，100足够。
  char invalue[CmdStr.CmdCount()][101];
  memset(invalue,0,sizeof(invalue));

  // 从http的GET请求报文中解析出输入参数，绑定到SQL中。
  for (int ii=0;ii<CmdStr.CmdCount();ii++)
  { 
    getvalue(buffer,CmdStr.m_vCmdStr[ii].c_str(),invalue[ii],100);
		
		logfile.Write("invalue[i] = %s\n\n",invalue[ii]);
    stmt.bindin(ii+1,invalue[ii],100);
  }

	 ////////////////////////////////////////

  ////////////////////////////////////////
  // 拆分colstr，可以得到结果集的字段数。
  CmdStr.SplitToCmd(colstr,",");

  // 用于存放结果集的数组。
  char colvalue[CmdStr.CmdCount()][2001];

  // 把结果集绑定到colvalue数组。
  for (int ii=0;ii<CmdStr.CmdCount();ii++)
  {
    stmt.bindout(ii+1,colvalue[ii],2000);
  }
  ////////////////////////////////////////
	

	char strsendbuffer[4001];
  memset(strsendbuffer,0,sizeof(strsendbuffer));
	
	if (stmt.execute() != 0)
  {
  	sprintf(strsendbuffer,"<retcode>%d</retcode><message>%s</message>\n",stmt.m_cda.rc,stmt.m_cda.message); 	
		
		Writen(sockfd,strsendbuffer,strlen(strsendbuffer));
		logfile.Write("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message);
		return false;

  }

	strcpy(strsendbuffer,"<retcode>0</retcode><message>ok</message>\n");
	Writen(sockfd,strsendbuffer,strlen(strsendbuffer));

  // logfile.WriteEx("<data>\n");
  Writen(sockfd,"<data>\n",strlen("<data>\n"));

  char strtemp[2001];

  // 逐行获取结果集，发送给客户端。
  while (true)
  {
    memset(strsendbuffer,0,sizeof(strsendbuffer));
    memset(colvalue,0,sizeof(colvalue));

    if (stmt.next() != 0) 
		{	
			logfile.Write("问题是：%s\n\n",stmt.m_cda.message);
			logfile.Write("colvalue =%s\n",colvalue);
			break;
	}

    for (int ii=0;ii<CmdStr.CmdCount();ii++)
    {
      memset(strtemp,0,sizeof(strtemp));
      snprintf(strtemp,2000,"<%s>%s</%s>",CmdStr.m_vCmdStr[ii].c_str(),colvalue[ii],CmdStr.m_vCmdStr[ii].c_str());
      strcat(strsendbuffer,strtemp);
      // logfile.WriteEx("<%s>%s</%s>",CmdStr.m_vCmdStr[ii].c_str(),colvalue[ii],CmdStr.m_vCmdStr[ii].c_str());
    }

    // logfile.WriteEx("<endl/>\n");
    strcat(strsendbuffer,"<endl/>\n");
    Writen(sockfd,strsendbuffer,strlen(strsendbuffer));
  }

  //logfile.WriteEx("</data>\n");
  Writen(sockfd,"</data>\n",strlen("</data>\n"));

  logfile.Write("intername=%s,count=%d\n",intername,stmt.m_cda.rpc);

	return true;
}





connpool::connpool()
{
	m_maxconns=0;
	m_timeout=0;
	memset(m_connstr,0,sizeof(m_connstr));
	memset(m_charset,0,sizeof(m_charset));
	m_conn=0;
}

connpool::~connpool()
{
	destroy();
}	
// 初始化数据库连接池
bool connpool::init(char * connstr,char * charset,const int maxconns,int timeout)
{
	// 尝试连接数据库
	connection conn;
	if(conn.connecttodb(connstr,charset)!=0)
	{
		printf("连接数据库失败，\n%s\n",conn.m_cda.message); return false;
	}
	conn.disconnect();

	strncpy(m_connstr,connstr,100);
	strncpy(m_charset,charset,100);
	m_maxconns = maxconns;
	m_timeout = timeout;
	
	// 分配数据库连接池的内存空间
	m_conn = new struct st_conn[m_maxconns];

	for(int i=0;i<m_maxconns;i++)
	{
		pthread_mutex_init(&m_conn[i].mutex,0);
		m_conn[i].atime=0;
	}

	return true;
}

// 断开数据库连接
void connpool:: destroy()
{
	for(int i=0;i<m_maxconns;i++)
	{
		m_conn[i].conn.disconnect();							// 断开数据库连接
		pthread_mutex_destroy(&m_conn[i].mutex);	// 销毁锁

	}

	delete []m_conn;
	m_conn=0;

	memset(m_connstr,0,sizeof(m_connstr));
	memset(m_charset,0,sizeof(m_charset));
	m_maxconns=0;
	m_timeout=0;
	
}

// 从数据库连接池中获取一个空闲,已连接好的connection，找到的话返回它的地址
// 如果没有找到，在连接池中找一个未连接数据库的connection，连接数据库，返回地址
// 连接失败返回空
// 如果连接池已经用完或连接数据库失败，返回空
connection *connpool:: get()
{	
	int pos =-1;
	for(int i=0;i<m_maxconns;i++)	
	{
		if(pthread_mutex_trylock(&m_conn[i].mutex)==0)		// 找到空闲且连接好的位置
		{
			if(m_conn[i].atime >0)  // 数据库已经是连接的状态
			{
				printf("连接到%d.\n",i);
				m_conn[i].atime=time(0);
				return &m_conn[i].conn;
			}
		
			if(pos==-1) pos =i;										// 记录第一个未连接数据库的数组位置
			else pthread_mutex_unlock(&m_conn[i].mutex);
		}
	}
	if(pos==-1) 
	{
		printf("连接池已经用光。\n");
		return NULL;
		
	}

	// 连接池没有用完，让m_conn[pos].conn连上数据库
	printf("新连接%d,\n",pos);
	
	// 连接数据库
	if(m_conn[pos].conn.connecttodb(m_connstr,m_charset)!=0)
	{
		printf("连接数据库失败\n");
		pthread_mutex_unlock(&m_conn[pos].mutex);
		return NULL;
	}

	m_conn[pos].atime=time(0);
	return &m_conn[pos].conn;
}


// 归还数据库连接
bool connpool::free(connection*conn)
{	
	for(int i =0;i<m_maxconns;i++)
	{	
		if(&m_conn[i].conn==conn)
		{
			printf("归还%d\n",i);
			m_conn[i].atime = time(0);
			pthread_mutex_unlock(&m_conn[i].mutex);
			return true;
		}
	}
	
	return false;
}
// 检查数据库连接池，断开空闲的连接，在服务程序中，用一个专用的子线程调用这个函数
void connpool::checkpool()
{
	for(int i =0;i<m_maxconns;i++)
	{
		if(pthread_mutex_trylock(&m_conn[i].mutex)==0)
		{
			if(m_conn[i].atime>0)
			{
				// 判断是否超时
				if(time(0)-m_conn[i].atime>m_timeout)
				{
					printf("连接%d已经超时\n",i);
					m_conn[i].atime = time(0);
					m_conn[i].conn.disconnect();
				}
				else
				{	
					// 如果没有超时，执行一次sql，检查连接是否有效，无效的话断开它
					// 如果网络断开了，或者重启了，那么需要重连数据库，在这里，只要断开连接就行了
					// 重连的操作交给get函数
					if( m_conn[i].conn.execute("select * from dual")!=0)
					{
						 printf("连接%d已经故障\n",i);
						 m_conn[i].atime = time(0);
						 m_conn[i].conn.disconnect();
					}
					}
				}
			}
		
		pthread_mutex_unlock(&m_conn[i].mutex);
		}

		// 如果尝试加锁失败，表示数据库正在使用
	}



void *checkpool(void *arg)				// 检查线程函数
{
	while(true)
	{
		oraconnpool.checkpool();
		sleep(30);
	}
}
