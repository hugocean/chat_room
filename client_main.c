#include "client.h"

pthread_mutex_t mutex;  //互斥量
pthread_cond_t cond;	//条件变量
int sockfd;			
pthread_t tid_send;		//线程号	
pthread_t tid_recv;		//线程号
time_t tt;
FILE *fp;
//char version;
char user_level[16];
int outputFd;



/*
作者：周海阳
日期：2018.9.6
函数名：void *SendMsg(void *arg)
描述：用来发送数据给服务器端信息的线程
参数：void *arg 用来传递初始化的socket的文件描述符
返回值：无
*/
void *SendMsg(void *arg)
{
	int ret;
	int SockFd = *(int *)arg;
	
	while(1)
	{
//		pthread_mutex_lock(&mutex);
		char choise[20];
		printf("1.register	\n");
		printf("2.login		\n");
		printf("3.exit		\n");
		scanf("%s", choise);
		switch(atoi(choise))
		{
			case 1:
				ret = UserReg(SockFd);
				if(ret == FAILURE)
				{
					printf("register failure \n");
				}
				else
				{
					printf("register success \n");
				}

				break;

			case 2:
				ret = UserLogin(SockFd);
				if(ret == FAILURE)
				{
					printf("login failure \n");
				}
				else
				{
					printf("login success \n");
					IntoMyZone(SockFd);
				}
				break;
		
			case 3:
				ret = Quit(SockFd);
				if(ret == FAILURE)
				{
					printf("quit failure \n");
				}
				else
				{
					printf("quit success \n");
				}
				break;
		
		}
	}

}
/*
作者：周海阳
日期：2018.9.6
函数名：void *RecvMsg(void *arg)
描述：负责接收来自服务器端信息的线程
参数：void *arg 用来传递初始化的socket的文件描述符
返回值：无
*/
void *RecvMsg(void *arg)
{
	int ret;
	int old;
	int fd = *(int *)arg;
	Info1 RecvInfo;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old);
	pthread_cond_wait(&cond, &mutex);
	while(1)
	{
		memset(&RecvInfo, 0, sizeof(RecvInfo));
		
		ret = recv(fd, &RecvInfo, sizeof(RecvInfo), 0);
		if(ret < 0)
		{
			perror("recv");
		}
	
		if(strcmp(RecvInfo.buf, "checkon") == 0)
		{
			printf("this is online frend : ");
			printf("name : %s \n", RecvInfo.name);
		}
		
		//私聊成功显示对方发送来的消息并写道文本
		else if(strcmp(RecvInfo.buf, "private") == 0)
		{
			tt = time(NULL);
			printf("you have a new message(private) %s  ", ctime(&tt));
			fprintf(fp, "private  %s \n", ctime(&tt));
			printf("from : %s \n", RecvInfo.name);
			fprintf(fp, "from : %s \n", RecvInfo.name);
			printf("recv : %s \n", RecvInfo.mesg);
			fprintf(fp, "recv : %s \n\n", RecvInfo.mesg);
		}
		//私聊失败
		else if(strcmp(RecvInfo.buf, "privatefalse") == 0)
		{
			printf("user %s not exit \n", RecvInfo.toname);
		}
		//文件传输失败
		else if(strcmp(RecvInfo.buf, "filefalse") == 0)
		{
			printf("user %s not exit \n", RecvInfo.toname);
		}
		//群聊成功显示群聊消息，并写道文本
		else if(strcmp(RecvInfo.buf, "group") == 0)
		{
			tt = time(NULL);
			printf("you have a new message(group)  %s ", ctime(&tt));
			fprintf(fp, "group %s \n", ctime(&tt));
			printf("from : %s \n", RecvInfo.name);
			fprintf(fp, "from : %s \n", RecvInfo.name);
			printf("recv : %s \n", RecvInfo.mesg);
			fprintf(fp, "recv : %s \n\n", RecvInfo.mesg);
		}
		//查看聊天室在线成员
		else if(strcmp(RecvInfo.buf, "roomcheckon") == 0)
		{
			printf("this is online frend : ");
			printf("name : %s \n", RecvInfo.name);
		}
		//聊天室成员群聊成功，显示并保存到文本
		else if(strcmp(RecvInfo.buf, "roomchat") == 0)
		{
			tt = time(NULL);
			printf("you have a new message(group) from chatroom %s : %s ", RecvInfo.group, ctime(&tt));
			fprintf(fp, "chatroom %s %s \n", ctime(&tt), RecvInfo.group);
			printf("from : %s \n", RecvInfo.name);
			fprintf(fp, "from : %s \n", RecvInfo.name);
			printf("recv : %s \n", RecvInfo.mesg);
			fprintf(fp, "recv : %s \n\n", RecvInfo.mesg);
		}
		//聊天室成员被踢出，没实现
		else if(strcmp(RecvInfo.buf, "roomquit") == 0)
		{
			pthread_cond_signal(&cond);
			IntoMyZone(sockfd);
		}
		else if(strcmp(RecvInfo.buf, "roomkick") == 0)
		{
			pthread_cond_signal(&cond);
			printf("you are kicked by admin \n");
			IntoMyZone(sockfd);
		}
		//挂起接收线程，让发送线程接收
		else if(strcmp(RecvInfo.buf, "wait") == 0)
		{
			printf("recv in wait \n");
			pthread_cond_wait(&cond, &mutex);
		}
		//上线提醒
		else if(strcmp(RecvInfo.buf, "welcome") == 0)
		{
			printf("user %s login \n", RecvInfo.name);
		}
		//下线提醒
		else if(strcmp(RecvInfo.buf, "quitsend") == 0)
		{
			printf("user %s logout \n", RecvInfo.name);
		}
		//成为管理员
		else if(strcmp(RecvInfo.buf, "beadmin") == 0)
		{
			printf("you are limited or have be admin  \n");
		}
		//成为普通用户
		else if(strcmp(RecvInfo.buf, "benormal") == 0)
		{
			printf("you are limited or hanv be normal \n");
		}
		//成为聊天室管理员失败，你已经是管理员
		else if(strcmp(RecvInfo.buf, "roomadmin") == 0)
		{
			printf("you are have be admin			 \n");
		}
		//聊天室被踢失败，没有管理员权限
		else if(strcmp(RecvInfo.buf, "roomkick") == 0)
		{
			printf("you are not admin can't limit	 \n");
		}
		//成为管理员成功
		else if(strcmp(RecvInfo.buf, "chlevel") == 0)
		{
			memset(user_level, 0, sizeof(user_level));
			printf("become admin ....\n");
			strcpy(user_level, "admin");
			printf("become success \n");
		}
		//成为普通用户成功
		else if(strcmp(RecvInfo.buf, "chnormal") == 0)
		{
			memset(user_level, 0, sizeof(user_level));
			printf("become normal ....\n");
			strcpy(user_level, "normal");
			printf("become success \n");
		}
		//被管理员禁言提醒
		else if(strcmp(RecvInfo.buf, "limited") == 0)
		{
			memset(user_level, 0, sizeof(user_level));
			printf("you are be limited by admin \n");
			strcpy(user_level, "limit");
			printf("please shua xin \n");
		}
		//接收其他用户发送来的文件
		else if(strcmp(RecvInfo.buf, "sendfile") == 0)
		{			
			int	openFlags;
    		mode_t filePerms;
    		char buf[64];
  			openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    		filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH;     
  		    outputFd = open("recvfile", openFlags, filePerms);
    		ret = lseek(outputFd, 64, SEEK_END);
			printf("ret = %d \n",ret);
			write(outputFd, RecvInfo.mesg, sizeof(RecvInfo.mesg));
			
			/*if (write(outputFd, RecvInfo.mesg, sizeof(RecvInfo.mesg)) != sizeof(RecvInfo.mesg))
			{
				printf("error \n");
			}
			else
			{
				printf("write \n");
			}
*/
		//	close(outputFd);

		/*	FILE *fd;
			fd = fopen("recvfile", "a");
			fwrite(&RecvInfo.mesg, sizeof(RecvInfo.mesg), 1, fd);
			fclose(fd);
		*/
		}
		//接收到其他的东西
		else
		{
			printf("not info \n");
		}
	}

}
/*
作者：周海阳
日期：2018.9.6
函数名：int main()
描述：主函数:负责初始化，创建两个线程
参数：无
返回值：0
*/
int main()
{
	int ret;

	signal(SIGINT, StopClient); 
	//声明信号SIGINT 当用Ctrl+c退出的时候，处理函数StopClient

	fp = fopen("./chat_records", "a+");
	sockfd = ClientInit();
	if(sockfd == FAILURE)
	{	
		printf("init failure \n");
		exit(1);
	}
	
	ret = pthread_create(&tid_send, NULL, SendMsg, &sockfd);
	if(ret != 0)
	{
		perror("pthread_creat");
		exit(1);
	}

	ret = pthread_create(&tid_recv, NULL, RecvMsg, &sockfd);
	if(ret != 0)
	{
		perror("pthread_creat");
		exit(1);
	}


	while(1)
	{
	}

	return 0;
}





















