#include"_mysql.h"

int main()
{
  connection conn; //数据库连接类

  if(conn.connecttodb("127.0.0.1,root,20020914,mysql,3306","utf8")!=0)
  { printf("connect falied\n%s\n",conn.m_cda.message); return -1;}



	struct st_girls
	{
		long id;
		char name[31];
		double weight;
		char btime[20];
	}stgirls;

	sqlstatement stmt(&conn);

	stmt.prepare("\
	insert into girls(id,name,weight,btime) values(:1,:2,:3,str_to_date(:4,'%%Y-%%m-%%d %%H:%%i:%%s'))");
	printf("=%s=\n",stmt.m_sql);
	
	stmt.bindin(1,&stgirls.id);
	stmt.bindin(2,stgirls.name,30);
	stmt.bindin(3,&stgirls.weight);
	stmt.bindin(4,stgirls.btime,19);

	//模拟数据
	for(int i=0;i<5;i++)
	{
		memset(&stgirls,0,sizeof(struct st_girls));
		stgirls.id =i+1;
		sprintf(stgirls.name,"西斯%02dgirl",i+1);
		stgirls.weight =45.25+i;
		sprintf(stgirls.btime,"2021-08-25 10:33:%02d",i);

		if(stmt.execute()!=0)
		{
			printf("execute() failed \n%s\n%s\n",stmt.m_sql,stmt.m_cda.message);
			return -1;
		}

		printf("成功插入%ld条数据\n",stmt.m_cda.rpc);
	}

	conn.commit(); //提交事物


  return 0;
}
