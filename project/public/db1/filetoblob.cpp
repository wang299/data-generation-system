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

	stmt.prepare("update girls set pic=:1 where id =:2");
	printf("=%s=\n",stmt.m_sql);
	
	stmt.bindinlob(1,stgirls.pic,&stgirls.picsize);
	stmt.bindin(2,&stgirls.id);

	//模拟数据
	for(int i=1;i<3;i++)
	{
		memset(&stgirls,0,sizeof(struct st_girls));
		stgirls.id =i;

		//把图片的内容加载到pic
		if(i==1) stgirls.picsize=filetobuf("1.jpg",stgirls.pic);
		if(i==2) stgirls.picsize=filetobuf("2.jpg",stgirls.pic);


		if(stmt.execute()!=0)
		{
			printf("execute() failed \n%s\n%s\n",stmt.m_sql,stmt.m_cda.message);
			return -1;
		}

		printf("成功修改%ld条数据\n",stmt.m_cda.rpc);
	}

	conn.commit(); //提交事物


  return 0;
}
