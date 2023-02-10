#include "_public.h"

//全国气象站点参数结构体
struct st_stcode
{
	char provname[31]; // 省
	char obtid[11];		 // 站号
	char obtname[31];	 // 站名
	double lat;				 // 纬度
	double lon;				 // 精度
	double height;		 // 高度
};

// 存放全国气象站点参数结构体的容器
vector<struct st_stcode>vstcode;

// 将数据放入容器中
bool LoadSTCode(const char *inifile);

struct st_surfdata
{
  char obtid[11];      // 站点代码。
  char ddatetime[21];  // 数据时间：格式yyyymmddhh24miss
  int  t;              // 气温：单位，0.1摄氏度。
  int  p;              // 气压：0.1百帕。
  int  u;              // 相对湿度，0-100之间的值。
  int  wd;             // 风向，0-360之间的值。
  int  wf;             // 风速：单位0.1m/s
  int  r;              // 降雨量：0.1mm。
  int  vis;            // 能见度：0.1米。
};

vector<struct st_surfdata>vsurfdata;

// 模拟生成全国气象站点分钟观测数据,存放在vsurdata容器中
void CrtSurfData();

CLogFile logfile;

int main(int argc,char *argv[])
{
	if(argc!=4)
 	{	
		printf("重新输入参数\n");
		printf("格式：/project/idc1/bin/crtsurfdata3 /project/idc1/ini/stcode.ini /tem/surfdata /log/idc/crtsurfdata3.log\n");
		return -1;
  }

	if (logfile.Open(argv[3])==false)
	{
		printf("logfile.Open(%s) failed.\n",argv[3]);
		return -1;
	}

	logfile.Write("crtsurfdata3 开始运行。\n");

	// 把站点数据放入容器
	if(LoadSTCode(argv[1])==false) return -1;
	
	CrtSurfData();
	
	logfile.Write("crtsurfdata3 运行结束。\n");

	return 0;
}


bool LoadSTCode(const char *inifile)
{
	CFile File;
	// 打开站点参数文件
	if(File.Open(inifile,"r")==false)
	{
		logfile.Write("File.Open(%s) failed.\n",inifile);return false;
	}
	
	char strBuffer[301];
	CCmdStr CmdStr;
	struct st_stcode stcode;
	while(true)
	{
		// 从站点参数文件读取一行，如果已读取完，跳出循环
		if(File.Fgets(strBuffer,300,true)==false) break;
		
		// 把读取到的一行拆分
		CmdStr.SplitToCmd(strBuffer,",",true);
		if(CmdStr.CmdCount()!=6) continue;
		// 把站点参数的每一个数据保存到站点参数结构体中
		CmdStr.GetValue(0,stcode.provname,30);
		CmdStr.GetValue(1,stcode.obtid,10);
		CmdStr.GetValue(2,stcode.obtname,30);
		CmdStr.GetValue(3,&stcode.lat);
		CmdStr.GetValue(4,&stcode.lon);
		CmdStr.GetValue(5,&stcode.height);
		
		// 把站点参数结构体放入参数容器
		vstcode.push_back(stcode);
}
/*	for(int ii=0;ii<vstcode.size();ii++)
	
	{
		logfile.Write("province = %s\n",vstcode[ii].provname);
	}*/

	// 关闭文件
	return true;
}

void CrtSurfData()
{
	// 播随机数种子	
	srand(time(0));

  // 获取当前时间，当成观测时间
	char strddatetime[21];
	memset(strddatetime,0,sizeof(strddatetime));
	LocalTime(strddatetime,"yyyymmddhh24miss");
	struct st_surfdata stsurfdata;

	//遍历气象站点参数的vscode容器
	for(int i = 0;i<vstcode.size();i++)
	{
		memset(&stsurfdata,0,sizeof(st_surfdata));

		// 用随机数填充分钟观测数据的结构体
		strncpy(stsurfdata.obtid,vstcode[i].obtid,10);   // 站点代码
 	  strncpy(stsurfdata.ddatetime,strddatetime,14);  // 数据时间：格式yyyymmddhh24miss
  	stsurfdata.t = rand()%351;              // 气温：单位，0.1摄氏度。
  	stsurfdata.p = rand()%265+1000;               // 气压：0.1百帕。
  	stsurfdata.u = rand()%100+1;               // 相对湿度，0-100之间的值。
  	stsurfdata.wd = rand()%360;              // 风向，0-360之间的值。
  	stsurfdata.wf = rand()%150;              // 风速：单位0.1m/s
  	stsurfdata.r = rand()%16;               // 降雨量：0.1mm。
  	stsurfdata.vis = rand()%5001+100000;            // 能见度：0.1米。	

		// 把观测数据的结构体放入vsurfdata容器
		vsurfdata.push_back(stsurfdata);
		
	}

	//printf("aaa\n");
}
