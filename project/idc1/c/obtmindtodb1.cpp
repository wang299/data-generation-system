/*
 *  obtmindtodb.cpp，本程序用于把全国站点参数数据保存到数据库T_ZHOBTCODE表中。
 *  作者：吴从周。
*/
#include "_public.h"
#include "_mysql.h"


CLogFile logfile;

connection conn;

CPActive PActive;


bool _obtmindtodb(char *pathname,char *connstr,char *charset);

void EXIT(int sig);

int main(int argc,char *argv[])
{
  // 帮助文档。
  if (argc!=5)
  {
    printf("\n");
    printf("Using:./obtmindtodb inifile connstr charset logfile\n");

    printf("Example:/project/tools1/bin/procctl 120 /project/idc1/bin/obtmindtodb /idcdata/surfdata \"127.0.0.1,root,20020914,mysql,3306\" utf8 /log/idc/obtminttodb.log\n\n");

    printf("本程序用于把全国站点参数数据保存到数据库表中，如果站点不存在则插入，站点已存在则更新。\n");
    printf("inifile 站点参数文件名（全路径）。\n");
    printf("connstr 数据库连接参数：ip,username,password,dbname,port\n");
    printf("charset 数据库的字符集。\n");
    printf("logfile 本程序运行的日志文件名。\n");
    printf("程序每秒10运行一次，由procctl调度。\n\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程。
  // 但请不要用 "kill -9 +进程号" 强行终止。
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // 打开日志文件。
  if (logfile.Open(argv[4],"a+")==false)
  {
    printf("打开日志文件失败（%s）。\n",argv[4]); return -1;
  }

  //PActive.AddPInfo(30,"obtmindtodb");   // 进程的心跳，10秒足够。
  // 注意，在调试程序的时候，可以启用类似以下的代码，防止超时。
   PActive.AddPInfo(5000,"obtcodetodb");



	_obtmindtodb(argv[1],argv[2],argv[3]);
	/*
  // 连接数据库。
  if (conn.connecttodb(argv[2],argv[3])!=0)
  {
    logfile.Write("connect database(%s) failed.\n%s\n",argv[2],conn.m_cda.message); return -1;
  }

  logfile.Write("connect database(%s) ok.\n",argv[2]);

  // 提交事务。
  conn.commit();
	*/
  return 0;
}


void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d\n\n",sig);

  conn.disconnect();

  exit(0);
}

//业务处理主函数
bool _obtmindtodb(char *pathname,char *connstr,char *charset)
{
	//打开目录
	CDir Dir;
	if(Dir.OpenDir(pathname,"*.xml") ==false)	
	{
		logfile.Write("dir.open failed\n");
		return false;
	}

	while(true)
	{
		if(Dir.ReadDir()==false) break;
		logfile.Write("filename =%s\n",Dir.m_FullFileName);		
	
	/*
		while(true)
		{
			
		}
*/
	}

	return true;	
}




