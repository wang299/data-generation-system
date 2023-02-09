#include"_mysql.h"

int main()
{
  connection conn; //数据库连接类

  if(conn.connecttodb("127.0.0.1,root,20020914,mysql,3306","utf8")!=0)
  { printf("connect falied\n%s\n",conn.m_cda.message); return -1;}



	struct st_girls
	{
		long id;
		char pic[100000];			//图片内容
		unsigned long picsize;//图片占用的字节数
	}stgirls;

	sqlstatement stmt(&conn);

	stmt.prepare("select id,pic from girls where id in(1,2)");
	
	stmt.bindoutlob(2,stgirls.pic,100000,&stgirls.picsize);
	stmt.bindout(1,&stgirls.id);

	if(stmt.execute()!=0)
	{
		printf("execute() failed \n%s\n%s\n",stmt.m_sql,stmt.m_cda.message);
		return -1;
	}
	
	while(true)
	{
		memset(&stgirls,0,sizeof(st_girls));
		if(stmt.next()!=0) break;

		//生成文件名
		char filename[101]; memset(filename,0,sizeof(filename));
		sprintf(filename,"%d_out.jpg",stgirls.id);

		buftofile(filename,stgirls.pic,stgirls.picsize);
		
	}

	printf("本次查询了1次\n");

	printf("select tabke girls ok.\n");


  return 0;
}
