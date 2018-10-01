#include "server.h"

Node *first = NULL;
GNode *group = NULL;

/*
作者：周海阳
日期：2018.9.6
函数名：int main()
描述：主函数
参数：
返回值：0
*/
int main()
{
	int ret, i;
	int SockFd;
	int fd[SIZE] = {0};

	//signal(SIGINT, StopServer);

	ret = LinkInit(&first); //初始化软件链表
	if(ret == FAILURE)
	{
		printf("Link init failure \n");
	}
	else
	{
		printf("Link init success \n");
	
	}

	SockFd = ServerInit(); //初始化服务器
	if(SockFd == FAILURE)
	{
		printf("Server Init Failure \n");
	}
	else
	{
		printf("Server Init success \n");
	}

	ret = sqlite3_open("login.db",&ppdb); //打开数据库
	if(ret != SQLITE_OK)
	{
		perror("sqlite3_open");
		exit(1);
	}
	
	ret = LinkReadData(); //将数据库的信息填到链表中
	if(ret == FAILURE)
	{
		printf("Link read data failure \n");
	}
	else
	{
		printf("Link read data success \n");
	}

	
	while(1)
	{
		ret = ServerAccept(SockFd);
		if(ret == FAILURE)
		{
			printf("accept failure \n");
		}	
	}

	return 0;
}














