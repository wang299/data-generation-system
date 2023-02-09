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

CLogFile logfile;

int main(int argc,char *argv[])
{
	if(argc!=4)
 	{	
		printf("重新输入参数\n");
		printf("格式：/project/idc1/bin/crtsurfdata2 /project/idc1/ini/stcode.ini /tem/surfdata /log/idc/crtsurfdata2.log\n");
		return -1;
  }

	if (logfile.Open(argv[3])==false)
	{
		printf("logfile.Open(%s) failed.\n",argv[3]);
		return -1;
	}

	logfile.Write("crtsurfdata2 开始运行。\n");

	// 把站点数据放入容器
	if(LoadSTCode(argv[1])==false) return -1;
	
	logfile.Write("crtsurfdata2 运行结束。\n");

	
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
