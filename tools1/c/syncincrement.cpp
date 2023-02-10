/*
 *  程序名：syncincrement.cpp，本程序是数据中心的公共功能模块，采用增量的方法同步MySQL数据库之间的表。
 *  作者：吴从周。
*/
#include "_tools.h"

struct st_arg
{
  char localconnstr[101];  // 本地数据库的连接参数。
  char charset[51];        // 数据库的字符集。
  char fedtname[31];       // Federated表名。
  char localtname[31];     // 本地表名。
  char remotecols[1001];   // 远程表的字段列表。
  char localcols[1001];    // 本地表的字段列表。
  char where[1001];        // 同步数据的条件。
  char remoteconnstr[101]; // 远程数据库的连接参数。
  char remotetname[31];    // 远程表名。
  char remotekeycol[31];   // 远程表的键值字段名。
  char localkeycol[31];    // 本地表的键值字段名。
  int  maxcount;           // 每批执行一次同步操作的记录数。
	int  timetvl;						 // 运行的间隔时间
  int  timeout;            // 本程序运行时的超时时间。
  char pname[51];          // 本程序运行时的程序名。
} starg;

// 显示程序的帮助
void _help(char *argv[]);

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer);

CLogFile logfile;

connection connloc;   // 本地数据库连接。
connection connrem;   // 远程数据库连接。

// 业务处理主函数。
bool _syncincrement(bool &bcontinue);

// 从本地表starg.localtname获取自增字段的最大值，存放在maxkeyvalue中
long maxkeyvalue =0;
bool findmaxkey();

void EXIT(int sig);

CPActive PActive;

int main(int argc,char *argv[])
{
  if (argc!=3) { _help(argv); return -1; }

  // 关闭全部的信号和输入输出，处理程序退出的信号。
  CloseIOAndSignal();
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false)
  {
    printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
  }

  // 把xml解析到参数starg结构中
  if (_xmltoarg(argv[2])==false) return -1;

  PActive.AddPInfo(starg.timeout,starg.pname);
  // 注意，在调试程序的时候，可以启用类似以下的代码，防止超时。
  // PActive.AddPInfo(starg.timeout*100,starg.pname);

  if (connloc.connecttodb(starg.localconnstr,starg.charset) != 0)
  {
    logfile.Write("connect database(%s) failed.\n%s\n",starg.localconnstr,connloc.m_cda.message); EXIT(-1);
  }

	
	if(connrem.connecttodb(starg.remoteconnstr,starg.charset)!=0)
	{
		logfile.Write("connect database(%s) failed.\n%s\n",starg.remoteconnstr,connrem.m_cda.message);EXIT(-1);
	}

  // logfile.Write("connect database(%s) ok.\n",starg.localconnstr);

  // 如果starg.remotecols或starg.localcols为空，就用starg.localtname表的全部列来填充。
  if ( (strlen(starg.remotecols)==0) || (strlen(starg.localcols)==0) )
  {
    CTABCOLS TABCOLS;

    // 获取starg.localtname表的全部列。
    if (TABCOLS.allcols(&connloc,starg.localtname)==false)
    {
      logfile.Write("表%s不存在。\n",starg.localtname); EXIT(-1); 
    }

    if (strlen(starg.remotecols)==0)  strcpy(starg.remotecols,TABCOLS.m_allcols);
    if (strlen(starg.localcols)==0)   strcpy(starg.localcols,TABCOLS.m_allcols);
  }
	
	bool bcontinue = true;
  // 业务处理主函数。
	while(true)
	{
		if(_syncincrement(bcontinue)==false) EXIT(-1);

		if(bcontinue==false) sleep(starg.timetvl);

		PActive.UptATime();
	}

}

// 显示程序的帮助
void _help(char *argv[])
{
  printf("Using:/project/tools1/bin/syncincrement logfilename xmlbuffer\n\n");

	printf("Sample:/project/tools1/bin/procctl 10 /project/tools1/bin/syncincrement /log/idc/syncincrement_ZHOBTMIND2.log \"<localconnstr>192.168.159.128,root,20020914,mysql,3306</localconnstr><remoteconnstr>192.168.159.128,root,20020914,mysql,3306</remoteconnstr><charset>utf8</charset><remotetname>t_zhobtind1</remotetname><fedtname>lk_zhobtind1</fedtname><localtname>t_zhobtind2</localtname><remotecols>obtid,ddatetime,t,p,u,wd,wf,r,vis,upttime,keyid</remotecols><localcols>stid,ddatetime,t,p,u,wd,wf,r,vis,upttime,recid</localcols><remotekeycol>keyid</remotekeycol><localkeycol>recid</localkeycol><maxcount>300</maxcount><timetvl>2</timetvl><timeout>50</timeout><pname>syncincrement1_ZHOBTMIND2</pname>\"\n\n");

	printf("       /project/tools1/bin/procctl 10 /project/tools1/bin/syncincrement /log/idc/syncincrement_ZHOBTMIND3.log \"<localconnstr>192.168.159.128,root,20020914,mysql,3306</localconnstr><remoteconnstr>192.168.159.128,root,20020914,mysql,3306</remoteconnstr><charset>utf8</charset><remotetname>t_zhobtind1</remotetname><fedtname>lk_zhobtind1</fedtname><localtname>t_zhobtind3</localtname><remotecols>obtid,ddatetime,t,p,u,wd,wf,r,vis,upttime,keyid</remotecols><localcols>stid,ddatetime,t,p,u,wd,wf,r,vis,upttime,recid</localcols><where>and obtid like '54%%%%'</where><remotekeycol>keyid</remotekeycol><localkeycol>recid</localkeycol><maxcount>300</maxcount><timetvl>2</timetvl><timeout>50</timeout><pname>syncincrement1_ZHOBTMIND3</pname>\"\n\n");

  printf("本程序是数据中心的公共功能模块，采用增量的方法同步MySQL数据库之间的表。\n\n");

  printf("logfilename   本程序运行的日志文件。\n");
  printf("xmlbuffer     本程序运行的参数，用xml表示，具体如下：\n\n");

  printf("localconnstr  本地数据库的连接参数，格式：ip,username,password,dbname,port。\n");
  printf("charset       数据库的字符集，这个参数要与远程数据库保持一致，否则会出现中文乱码的情况。\n");

  printf("fedtname      Federated表名。\n");
  printf("localtname    本地表名。\n");

  printf("remotecols    远程表的字段列表，用于填充在select和from之间，所以，remotecols可以是真实的字段，\n"\
         "              也可以是函数的返回值或者运算结果。如果本参数为空，就用localtname表的字段列表填充。\n");
  printf("localcols     本地表的字段列表，与remotecols不同，它必须是真实存在的字段。如果本参数为空，\n"\
         "              就用localtname表的字段列表填充。\n");

  printf("where         同步数据的条件，填充在select remotekeycol from remotetname where remotekeycol>:1\n注意不要加where关键字\n"\
         "              之后，注意：1）where中的字段必须同时在本地表和Federated表中；2）不要用系统时间作\n"\
         "              为条件。\n");

  printf("remoteconnstr 远程数据库的连接参数，格式与localconnstr相同，。\n");
  printf("remotetname   远程表名。\n");
  printf("remotekeycol  远程表的键值字段名。\n");
  printf("localkeycol   本地表的键值字段名。\n");

  printf("maxcount      每批执行一次同步操作的记录数，不能超过MAXPARAMS宏（在_mysql.h中定义），当synctype==2时有效。\n");
	printf("timetvl       执行同步的时间间隔，单位秒\n");

  printf("timeout       本程序的超时时间，单位：秒，视数据量的大小而定，建议设置30以上。\n");
  printf("pname         本程序运行时的进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n");
}

// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer)
{
  memset(&starg,0,sizeof(struct st_arg));

  // 本地数据库的连接参数，格式：ip,username,password,dbname,port。
  GetXMLBuffer(strxmlbuffer,"localconnstr",starg.localconnstr,100);
  if (strlen(starg.localconnstr)==0) { logfile.Write("localconnstr is null.\n"); return false; }

  // 数据库的字符集，这个参数要与远程数据库保持一致，否则会出现中文乱码的情况。
  GetXMLBuffer(strxmlbuffer,"charset",starg.charset,50);
  if (strlen(starg.charset)==0) { logfile.Write("charset is null.\n"); return false; }

  // Federated表名。
  GetXMLBuffer(strxmlbuffer,"fedtname",starg.fedtname,30);
  if (strlen(starg.fedtname)==0) { logfile.Write("fedtname is null.\n"); return false; }

  // 本地表名。
  GetXMLBuffer(strxmlbuffer,"localtname",starg.localtname,30);
  if (strlen(starg.localtname)==0) { logfile.Write("localtname is null.\n"); return false; }

  // 远程表的字段列表，用于填充在select和from之间，所以，remotecols可以是真实的字段，也可以是函数
  // 的返回值或者运算结果。如果本参数为空，就用localtname表的字段列表填充。\n");
  GetXMLBuffer(strxmlbuffer,"remotecols",starg.remotecols,1000);

  // 本地表的字段列表，与remotecols不同，它必须是真实存在的字段。如果本参数为空，就用localtname表的字段列表填充。
  GetXMLBuffer(strxmlbuffer,"localcols",starg.localcols,1000);

  // 同步数据的条件，即select语句的where部分。
  GetXMLBuffer(strxmlbuffer,"where",starg.where,1000);


    // 远程数据库的连接参数，格式与localconnstr相同，当synctype==2时有效。
  GetXMLBuffer(strxmlbuffer,"remoteconnstr",starg.remoteconnstr,100);
  if (strlen(starg.remoteconnstr)==0) { logfile.Write("remoteconnstr is null.\n"); return false; }

    // 远程表名，当synctype==2时有效。
  GetXMLBuffer(strxmlbuffer,"remotetname",starg.remotetname,30);
  if (strlen(starg.remotetname)==0) { logfile.Write("remotetname is null.\n"); return false; }

    // 远程表的键值字段名，必须是唯一的，当synctype==2时有效。
  GetXMLBuffer(strxmlbuffer,"remotekeycol",starg.remotekeycol,30);
  if (strlen(starg.remotekeycol)==0) { logfile.Write("remotekeycol is null.\n"); return false; }

    // 本地表的键值字段名，必须是唯一的，当synctype==2时有效。
  GetXMLBuffer(strxmlbuffer,"localkeycol",starg.localkeycol,30);
  if (strlen(starg.localkeycol)==0) { logfile.Write("localkeycol is null.\n"); return false; }

    // 每批执行一次同步操作的记录数，不能超过MAXPARAMS宏（在_mysql.h中定义），当synctype==2时有效。
  GetXMLBuffer(strxmlbuffer,"maxcount",&starg.maxcount);
  if (starg.maxcount==0) { logfile.Write("maxcount is null.\n"); return false; }
  if (starg.maxcount>MAXPARAMS) starg.maxcount=MAXPARAMS;

  // 本程序的超时时间，单位：秒，视数据量的大小而定，建议设置30以上。
  GetXMLBuffer(strxmlbuffer,"timeout",&starg.timeout);
  if (starg.timeout==0) { logfile.Write("timeout is null.\n"); return false; }
	
	// 处理timetvl和timeout
	if(starg.timeout<starg.timetvl+10) starg.timeout = starg.timetvl+10;

  // 本程序运行时的进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。
  GetXMLBuffer(strxmlbuffer,"pname",starg.pname,50);
  if (strlen(starg.pname)==0) { logfile.Write("pname is null.\n"); return false; }
	
	// 执行同步的时间间隔，单位秒
	GetXMLBuffer(strxmlbuffer,"timetvl",&starg.timetvl);
	if(starg.timetvl>30) starg.timetvl=30;
  return true;
}

void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d\n\n",sig);

  connloc.disconnect();

  connrem.disconnect();

  exit(0);
}

/*
create table LK_ZHOBTCODE1
(
   obtid                varchar(10) not null comment '站点代码',
   cityname             varchar(30) not null comment '城市名称',
   provname             varchar(30) not null comment '省名称',
   lat                  int not null comment '纬度，单位：0.01度。',
   lon                  int not null comment '经度，单位：0.01度。',
   height               int not null comment '海拔高度，单位：0.1米。',
   upttime              timestamp not null comment '更新时间。',
   keyid                int not null auto_increment comment '记录编号，自动增长列。',
   primary key (obtid),
   unique key ZHOBTCODE1_KEYID (keyid)
)ENGINE=FEDERATED CONNECTION='mysql://root:mysqlpwd@192.168.174.132:3306/mysql/T_ZHOBTCODE1';

create table LK_ZHOBTMIND1
(
   obtid                varchar(10) not null comment '站点代码。',
   ddatetime            datetime not null comment '数据时间，精确到分钟。',
   t                    int comment '湿度，单位：0.1摄氏度。',
   p                    int comment '气压，单位：0.1百帕。',
   u                    int comment '相对湿度，0-100之间的值。',
   wd                   int comment '风向，0-360之间的值。',
   wf                   int comment '风速：单位0.1m/s。',
   r                    int comment '降雨量：0.1mm。',
   vis                  int comment '能见度：0.1米。',
   upttime              timestamp not null comment '更新时间。',
   keyid                bigint not null auto_increment comment '记录编号，自动增长列。',
   primary key (obtid, ddatetime),
   unique key ZHOBTMIND1_KEYID (keyid)
)ENGINE=FEDERATED CONNECTION='mysql://root:mysqlpwd@192.168.174.132:3306/mysql/T_ZHOBTMIND1';
*/

// 业务处理主函数。
bool _syncincrement(bool &bcontinue)
{
  CTimer Timer;
	
	bcontinue =false;
	// 从starg.localtname获取自增字段的最大值，存放在maxkeyvalue全局变量中
	
	//从远程表查找大于maxkeyvalue的值
	if(findmaxkey()==false)return false;

	char remkeyvalue[51];

	sqlstatement stmtsel(&connrem);
	stmtsel.prepare("select %s from %s where %s>:1 %s order by %s",starg.remotekeycol,starg.remotetname,starg.remotekeycol,starg.where,starg.remotekeycol);
	stmtsel.bindout(1,remkeyvalue,50);
	stmtsel.bindin(1,&maxkeyvalue);
	
	// 拼接绑定同步sql的语句参数
	char bindstr[2001];
	char strtemp[11];

	memset(bindstr,0,sizeof(bindstr));

	for(int i=0;i<starg.maxcount;i++)
	{
		memset(strtemp,0,sizeof(strtemp));
		sprintf(strtemp,":%lu,",i+1);
		strcat(bindstr,strtemp);
	}
	bindstr[strlen(bindstr)-1]=0;
	
	// 存放key字段的值
	char keyvalues[starg.maxcount][51];
	
  sqlstatement stmtins(&connloc);    // 执行向本地表中插入数据的SQL语句。

  // 准备插入目的表数据的sql，一次插入starg.maxcount条记录。
  stmtins.prepare("insert into %s(%s) select %s from %s where %s in (%s)",starg.localtname,starg.localcols,starg.remotecols,starg.fedtname,starg.remotekeycol,bindstr);
  
	for (int ii=0; ii<starg.maxcount; ii++)
  {
    stmtins.bindin(ii+1,keyvalues[ii],50);
  }

  int ccount=0;

  memset(keyvalues,0,sizeof(keyvalues));

	 if (stmtsel.execute() !=0)
   {
        // 执行从目的表中删除的操作一般不会有问题，如果报错，就一定是数据库的问题或sql语法的>问题，流程不必继续。
     logfile.Write("stmtsel.execute() failed.\n%s\n%s\n",stmtsel.m_sql,stmtsel.m_cda.message); return false;
   }

  while (true)
  {
    if (stmtsel.next() != 0) break;
		
		// 将数组中的数据复制到keyvalues中
    strncpy(keyvalues[ccount],remkeyvalue,50);

    ccount++;

    // 每starg.maxcount条记录执行一次同步语句。
    if (ccount == starg.maxcount)
    {
		
			 // 向目的表中插入记录。
      if (stmtins.execute() != 0)
      {
        // 不管报什么错，都写日志。
        logfile.Write("stmtins.execute() failed.\n%s\n%s\n",stmtins.m_sql,stmtins.m_cda.message);

        // 如果不是键值冲突，就一定是数据库的问题或sql语法的问题，流程不必继续。
      }

      logfile.Write("sync %s to %s(%d rows) in %.2fsec.\n",starg.remotetname,starg.localtname,ccount,Timer.Elapsed());

      connloc.commit();

      memset(keyvalues,0,sizeof(keyvalues));

      ccount=0;
			PActive.UptATime();
    }
  }

  // 如果不足starg.maxcount条记录，再执行一次同步。
  if (ccount > 0)
  {
    // 向目的表中插入记录。
    if (stmtins.execute() != 0)
    {
      // 不管报什么错，都写日志。
      logfile.Write("stmtins.execute() failed.\n%s\n%s\n",stmtins.m_sql,stmtins.m_cda.message);

      // 如果不是键值冲突，就一定是数据库的问题或sql语法的问题，流程不必继续。
      if (stmtins.m_cda.rc != 1062) return false;
    }

		logfile.Write("sync %s to %s(%d rows) in %.2fsec.\n",starg.remotetname,starg.localtname,ccount,Timer.Elapsed());
	}


	if(stmtsel.m_cda.rpc>0) 
	{	
		bcontinue =true;
  	//logfile.Write("sync %s to %s(%d rows) in %.2fsec.\n",starg.remotetname,starg.localtname,stmtsel.m_cda.rpc,Timer.Elapsed());	
	}
	return true;
}

bool findmaxkey()
{
	maxkeyvalue =0;

	sqlstatement stmt(&connloc);
	stmt.prepare("select max(%s) from %s",starg.localkeycol,starg.localtname);
	stmt.bindout(1,&maxkeyvalue);

	if(stmt.execute()!=0)
	{
		logfile.Write("stmt.execute failed \n%s\n%s\n",stmt.m_sql,stmt.m_cda.message);
		return false;
	}
	stmt.next();


	//logfile.Write("maxkeyvalue =%ld\n",maxkeyvalue);

	return true;
}

