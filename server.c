#include "server.h"

int ret;
extern Node *first;  	
//定义软件链表
extern GNode *group;	
//定义聊天室链表
char groupname[32] = {0};
//聊天室的名称
char null[32] = {0};
char buf[10] = "false";
char buffer[10] = "success";
char buff[10] = "failure";


/*
作者：周海阳
日期：2018.9.6
函数名：int ServerInit()
描述：服务器初始化：创建套接字；bind绑定；listen监听
参数：套接字的文件描述符
返回值：socket套接字的文件描述符
*/
int ServerInit()
{
	int sockfd;
	
	//创建socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(-1 == sockfd)
	{
		perror("socket");
		exit(1);
	}

	//绑定
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("192.168.238.129");
	ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret < 0)
	{
		perror("bind");
		exit(1);
	}

	//监听
	ret = listen(sockfd, 5);
	if(ret < 0)
	{
		perror("listen");
		exit(1);
	}

	return sockfd;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int ServerAccept(int sockFd)
描述：accept接收来自客户端的连接，没接收到一个连接就起一个线程处理之间的消息通信
参数：套接字的文件描述符
返回值：SUCCESS：成功
		FAILURE：失败
*/
int ServerAccept(int sockFd)
{
	pthread_t tid;
	int fd[SIZE];
	int i = 0;
	int length = sizeof(client_addr);
	
	//监听
	fd[i] = accept(sockFd, (struct sockaddr *)&client_addr, &length);	
	if(fd[i] == -1)
	{
		perror("accept");
	}
	else
	{
		printf("client connect success \n");
		printf("port is %d fd is %d \n", client_addr.sin_port, fd[i]);	
	}

	printf("creat pthread \n");
	//创建线程
	ret = pthread_create(&tid, NULL, ClientHandler, &fd[i]);
	if(ret != 0)
	{
		perror("pthread_creat");
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：void *ClientHandler(void *arg)
描述：线程处理函数
参数：套接字的文件描述符
返回值：无
*/
void *ClientHandler(void *arg)
{
	Info1 RecvInfo;  //用来接收服务器发送来的结构体
	int fd = *(int *)arg;
	char buf_name[10] = "name";
	char buf_id[10] = "id";
	char buf_passwd[10] = "passwd";

	//线程分离
	pthread_detach(pthread_self());
	while(1)
	{
		//不停的接收结构体
		ret = recv(fd ,&RecvInfo, sizeof(RecvInfo), 0);
		if(ret == -1)
		{
			perror("recv");
			exit(1);
		}
		
		//通过判断结构体的buf数组来做不同的操作
		if(strcmp(RecvInfo.buf, "register") == 0) //服务器接收到注册请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv id     : %s \n", RecvInfo.id);
			printf("recv passwd : %s \n", RecvInfo.passwd);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			
			//判断姓名是否重名，
			if(RepetitionName(first, RecvInfo.name) == TRUE) 
			{
				send(fd, buf, sizeof(buf), 0);
				continue;
			}

			//判断id是否重名
			if(RepetitionId(first, RecvInfo.id) == TRUE)
			{
				send(fd, buf, sizeof(buf), 0);
				continue;
			}

			//如果没问题就把flag状态为写为register，存到数据库和链表中
			strcpy(RecvInfo.flag, "register"); 

			ret = DataWrite(RecvInfo, fd);  //将注册信息写道数据库中
			if(ret == SUCCESS)
			{
				printf("register info write in datebase success \n");
			}
			else
			{
				printf("register info write in datebase failure \n");
			}

			
			ret = LinkInsert(first, &RecvInfo, fd); //将信息插入到链表中
			if(ret == SUCCESS)
			{
				printf("register info write in link success \n");
			}
			else
			{
				printf("register info write in link failure \n");
			}
			
			printf("this is link info \n");
			
			ret = LinkShow(first); //显示链表信息
			if(ret == SUCCESS)
			{
				printf("link show success \n");
			}
			else
			{
				printf("link show failure \n");
			}
			
			printf("this is data info \n");

			ret = DataShow(); //显示数据库信息
			if(ret == SUCCESS)
			{
				printf("data show success \n");
			}
			else
			{
				printf("data show failure \n");
			}

			send(fd, buffer, sizeof(buffer), 0);
			
			memset(&RecvInfo, 0, sizeof(RecvInfo));
		}
 
		else if(strcmp(RecvInfo.buf, "login") == 0)  //服务器接收到登陆请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv id     : %s \n", RecvInfo.id);
			printf("recv passwd : %s \n", RecvInfo.passwd);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			
			//判断之前用户有没有登陆过
			if(LoginCheck(first, RecvInfo.name, RecvInfo.id, RecvInfo.passwd) == TRUE && LoginCheckon(first, RecvInfo.name) == TRUE)
			{
				//如果之间没有登陆过就发送登陆成功标志

				ret = SendLogin(first, RecvInfo.name, "success", fd);
				if(ret == SUCCESS)
				{
					printf("login success \n");
				}
				else
				{
					printf("login failure \n");
				}
				
				LinkChange(first, &RecvInfo, "login"); 
				DataChange(RecvInfo, "login");
				
				//将文件描述符写入数据库和链表
				LinkChangeFd(first, &RecvInfo, fd);
				DataChangeFd(RecvInfo, fd);		
				
				//发送消息给其他在线用户，通知他们有用户登陆
				ret = SendWelcome(first, RecvInfo.name);
				if(ret == SUCCESS)
				{
					printf("success \n");
				}
				else
				{
					printf("failure \n");
				}
			}
			else
			{	
				//如果之前登陆过就将告诉客户端登陆失败
				ret = SendLogin(first, RecvInfo.name, "failure", fd);
				if(ret == SUCCESS)
				{
					printf("login success \n");
				}
				else
				{
					printf("login failure \n");
				}
				/*ret = send(fd, buf, sizeof(buf), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}*/
			}

		}

		else if(strcmp(RecvInfo.buf, "quit") == 0) //服务器接收到退出请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv id     : %s \n", RecvInfo.id);
			printf("recv passwd : %s \n", RecvInfo.passwd);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			
			//发送消息给其他在线用户，通知他们有用户下线
			ret = SendQuit(first, RecvInfo.name);
			if(ret == SUCCESS)
			{
				printf("success \n");
			}
			else
			{
				printf("failure \n");
			}

			//将数据库和链表中用户的状态位改为quit
			LinkChange(first, &RecvInfo, "quit");
			DataChange(RecvInfo, "quit");
			
			//将数据库和链表中用户的文件描述符改为0
			LinkChangeFd(first, &RecvInfo, 0);
			DataChangeFd(RecvInfo, 0);
			
			//关闭线程
			pthread_exit(0);
			
		}
		else if(strcmp(RecvInfo.buf, "checkon") == 0) //查看在线成员请求
		{

			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv id     : %s \n", RecvInfo.id);
			printf("recv passwd : %s \n", RecvInfo.passwd);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
		
			//查看在线成员
			ret = CheckOn(first, "login", fd);
			if(ret == SUCCESS)
			{	
				printf("check frind success \n");
			}
			else
			{
				printf("check frind failure \n");
		
			}
		}

		else if(strcmp(RecvInfo.buf, "private") == 0) //私聊请求
		{
			int tofd;
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv id     : %s \n", RecvInfo.id);
			printf("recv passwd : %s \n", RecvInfo.passwd);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
	
			LinkShow(first);

			//先在链表中遍历成员找出要私聊用户的文件描述符，用来发送
			//成功返回文件描述符
			//失败返回FAILURE
			tofd = GetFd(first, RecvInfo.toname);
			if(tofd != FAILURE)
			{
				printf("get fd success : %d \n", tofd);
				//将私聊消息转发到用户
				ret = send(tofd, &RecvInfo, sizeof(RecvInfo), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}
			}
			else
			{
				//没找到说明用户发的昵称不存在，服务器将发送私聊失败的消息给用户
				strcpy(RecvInfo.buf, "privatefalse");
				tofd = GetFd(first, RecvInfo.name);
				ret = send(tofd, &RecvInfo, sizeof(RecvInfo), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}
				printf("failure \n");
			}
		}
		else if(strcmp(RecvInfo.buf, "group") == 0) //和在线成员群聊请求
		{
			char *name;
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv id     : %s \n", RecvInfo.id);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			LinkShow(first);

			//在链表中找到状态位为login的所有用户，将消息一个个的发送给他们
			ret = GroupChat(first, "login", RecvInfo.name, RecvInfo.mesg);
			if(ret == SUCCESS)
			{
				printf("send success \n");
			}
			else
			{
				printf("send failure \n");
			}
		}
		else if(strcmp(RecvInfo.buf, "creatgroup") == 0) //创建聊天室请求
		{
			Info1 TempInfo;
			//先发送等待给用户端的接收进程使他挂起
			strcpy(TempInfo.buf, "wait");		
			ret = send(fd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send wait failure \n");
			}
			else
			{
				printf("send wait success \n");
			}
			
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			
			//判断聊天室是否已经创建过
			if(strcmp(groupname, null) == 0)
			{
				//如果之前没创建过聊天室把全局变量的聊天室名字赋值
				strcpy(groupname, RecvInfo.group);
				printf("group name is : %s\n", groupname);
				//发送创建成功消息给用户
				send(fd, buffer, sizeof(buffer), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}
				
				ret = GroupLinkInit(&group);  //聊天室链表初始化
				if(ret == FAILURE)
				{
					printf("Group Link init failure \n");
				}		
				else
				{
					printf("Group Link init success \n");
				}
			
				ret = GroupLinkShow(group); //显示链表信息
				if(ret = SUCCESS)
				{
					printf("this is group link list \n");
				}
				else
				{
					printf("show failure \n");
				}
			}
			else
			{
				//之前创建过聊天室则发送给客户端创建失败消息
				send(fd, buf, sizeof(buf), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}				
			}
		}
		else if(strcmp(RecvInfo.buf, "entergroup") == 0) 	//进入聊天室请求
		{
			Info1 TempInfo;
			//先发送等待给用户端的接收进程使他挂起
			strcpy(TempInfo.buf, "wait");
			ret = send(fd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send wait failure \n");
			}
			else
			{
				printf("send wait success \n");
			}

			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);

			//判断聊天的名字是否正确
			if(strcmp(groupname, RecvInfo.group) == 0) 
			{
				//发送进入成功给用户
				send(fd, buffer, sizeof(buffer), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}

				//把用户信息插入到聊天室链表
				ret = GroupLinkInsert(group, &RecvInfo, fd, "normal");
				if(ret == SUCCESS)
				{
					printf("group info write in link success \n");
				}
				else
				{
					printf("group info write in link failure \n");
				}
				
				//显示聊天室用户信息
				ret = GroupLinkShow(group);
				if(ret = SUCCESS)
				{
					printf("this is group link list \n");
				}
				else
				{
					printf("show failure \n");
				}
			}
			else
			{
				//发送进入失败给用户
				send(fd, buf, sizeof(buf), 0);
				if(ret == -1)
				{
					perror("send");
				}
			
			}
		}
		else if(strcmp(RecvInfo.buf, "roomcheckon") == 0) //查看聊天室成员请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			
			//查看聊天室成员并发送给查看用户
			ret = RoomCheckon(group, "online", fd);
			if(ret = SUCCESS)
			{
				printf("chekc on success \n");
			}
			else
			{
				printf("check on failure \n");
			}

		}
		else if(strcmp(RecvInfo.buf, "roomchat") == 0) //和聊天室成员进行群聊请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
		
			//找到聊天室链表中状态位为online的成员并一一发送消息
			ret = RoomChat(group, "online", RecvInfo.name, RecvInfo.group, RecvInfo.mesg);
			if(ret == SUCCESS)
			{
				printf("send success \n");
			}
			else
			{
				printf("send failure \n");
			}

		}
		else if(strcmp(RecvInfo.buf, "roomkick") == 0) //踢出聊天室成员请求
		{
			Info1 TempInfo;
			int tofd;
			//先发送等待消息给用户端的接收进程使他挂起
			strcpy(TempInfo.buf, "wait");		
			ret = send(fd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send wait failure \n");
			}
			else
			{
				printf("send wait success \n");
			}
			
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			
			//先判断该人是否为管理员
			ret = CkeckAdmin(group, RecvInfo.name); 
			if(ret == TRUE)
			{
				printf("this is admin \n");
				
				//发送成功给用户，你可以踢人
				send(fd, buffer, sizeof(buffer), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}

				//遍历链表成员找出被踢用户的文件描述符
				tofd = GetFd(first, RecvInfo.rmname);
				if(tofd != FAILURE)
				{
					//把被踢用户的状态标志位写为quit
					ret = GroupLinkFlag(group, RecvInfo.rmname, "quit");
					if(ret == SUCCESS)
					{
						printf("quit success \n");
					}

					printf("get fd success %d \n", tofd);
					//把接收到的结构体转发给被踢用户
					ret = send(tofd, &RecvInfo, sizeof(RecvInfo), 0);
					if(ret == -1)
					{
						perror("send");
					}
					else
					{
						printf("send success \n");
					}
				}
				else
				{
					printf("failure \n");
				}

			}
			else
			{
				printf("this is not admin \n");
				//不是管理员就不能踢人
				send(fd, buf, sizeof(buf), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}
			}

			ret = GroupLinkShow(group);
			if(ret = SUCCESS)
			{
				printf("this is group link list \n");
			}
			else
			{
				printf("show failure \n");
			}

		}
		else if(strcmp(RecvInfo.buf, "roomadmin") == 0) //晋升管理员请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			
			//先查看该用户是否为管理员
			ret = CheckNormal(group, RecvInfo.name);
			//如果是普通用户就让他升级为管理员
			if(ret == TRUE)
			{
				printf("not admin is normal\n");
				ret = GroupLinkRoot(group, RecvInfo.name, "admin");
				if(ret == SUCCESS)
				{
					printf("become admin success \n");
				}
			}
			//否则返回消息给用户
			else
			{
				printf("is admin or limit \n");
				ret = send(fd, &RecvInfo, sizeof(RecvInfo), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success 11 \n");
				}
				continue;
			}
		
		
			ret = GroupLinkShow(group);
			if(ret = SUCCESS)
			{
				printf("this is group link list \n");
			}
			else
			{
				printf("show failure \n");
			}
		

		}
		else if(strcmp(RecvInfo.buf, "roomquit") == 0) //退出聊天室请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
 			
			//遍历聊天室链表将该成员的状态位写为quit
			ret = GroupLinkFlag(group, RecvInfo.name, "quit");
			if(ret == SUCCESS)
			{
				printf("quit success \n");
			}

			ret = GroupLinkShow(group);
			if(ret = SUCCESS)
			{
				printf("this is group link list \n");
			}
			else
			{
				printf("show failure \n");
			}
			
		}
		else if(strcmp(RecvInfo.buf, "beadmin") == 0) //成为管理员请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
		
			ret = LinkShow(first); //显示链表信息
			if(ret == SUCCESS)
			{
				printf("link show success \n");
			}
			else
			{
				printf("link show failure \n");
			}
			
			printf("this is data info \n");

			ret = DataShow(); //显示数据库信息
			if(ret == SUCCESS)
			{
				printf("data show success \n");
			}
			else
			{
				printf("data show failure \n");
			}

			//首先查看用户是否为普通用户
			ret = CheckNormal(first, RecvInfo.name);
			//是普通用户就让他成为管理员
			if(ret == TRUE)
			{
				printf(" is normal\n");
				ret = DataLevel(RecvInfo, "admin");
				if(ret == SUCCESS)
				{
					printf("data change success \n");
				}
				else
				{
					printf("data change failure \n");
				}
		
				ret = ChangeLevel(first, RecvInfo.name, "admin");
				if(ret == SUCCESS)
				{
					printf("become admin \n");
					Info1 tempinfo;
					strcpy(tempinfo.buf, "chlevel");
					ret = send(fd, &tempinfo, sizeof(tempinfo), 0);
					if(ret == -1)
					{
						perror("send");
					}
					else
					{
						printf("send success 11 \n");
					}
				}
				else
				{
					printf("failure  \n");
				}
			}
			//否则返回消息给用户
			else
			{
				printf("is admin or limit \n");
				ret = send(fd, &RecvInfo, sizeof(RecvInfo), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success 11 \n");
				}
				continue;
			}
		

		}
		else if(strcmp(RecvInfo.buf, "limitsb") == 0) //禁言成员
		{
			Info1 TempInfo;
			Log tempinfo;
			int tofd;
			//先发送等待消息给用户端的接收进程使他挂起
			strcpy(TempInfo.buf, "wait");		
			ret = send(fd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send wait failure \n");
			}
			else
			{
				printf("send wait success \n");
			}
			
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			printf("recv limitsb: %s \n", RecvInfo.lmname);
	
			//首先判断用户是否为管理员，只有管理员才能禁言
			ret = CheckAdmin(first, RecvInfo.name);
			if(ret == TRUE)
			{
				printf("you are admin  you can limit \n");
				ret = ChangeLimit(first, RecvInfo.lmname);
				if(ret == TRUE)
				{
					strcpy(tempinfo.buf, "success");
					ret = send(fd, &tempinfo, sizeof(tempinfo), 0);
					if(ret == -1)
					{
						printf("send limit failure \n");
					}
					else
					{
						printf("send limit success \n");
					}

					tofd = GetFd(first, RecvInfo.lmname);
					strcpy(TempInfo.buf, "limited");		
					ret = send(tofd, &TempInfo, sizeof(TempInfo), 0);
					if(ret == -1)
					{
						printf("send limit ed failure \n");
					}
					else
					{
						printf("send limit ed success \n");
					}
					
				}
				else
				{
					strcpy(tempinfo.buf, "failure");
					ret = send(fd, &tempinfo, sizeof(tempinfo), 0);
					if(ret == -1)
					{
						printf("send limit failure \n");
					}
					else
					{
						printf("send limit success \n");
					}
				
				}
			}
			else
			{
				printf("you are not admin  you can not limit \n");
				strcpy(tempinfo.buf, "failure");
				ret = send(fd, &tempinfo, sizeof(tempinfo), 0);
				if(ret == -1)
				{
					printf("send limit failure \n");
				}
				else
				{
					printf("send limit success \n");
				}
			
			}

		}
		else if(strcmp(RecvInfo.buf, "benormal") == 0) //成为普通用户请求
		{
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
		
			//首先判断用户是否为管理员
			ret = CheckAdmin(first, RecvInfo.name);
			if(ret == TRUE)
			{
				printf(" is admin\n");
				ret = DataLevel(RecvInfo, "normal");
				if(ret == SUCCESS)
				{
					printf("data change success \n");
				}
				else
				{
					printf("data change failure \n");
				}
				ret = ChangeLevel(first, RecvInfo.name, "normal");
				if(ret == SUCCESS)
				{
					printf("become normal \n");
					Info1 tempinfo;
					strcpy(tempinfo.buf, "chnormal");
					ret = send(fd, &tempinfo, sizeof(tempinfo), 0);
					if(ret == -1)
					{
						perror("send");
					}
					else
					{
						printf("send success 11 \n");
					}
				}
				else
				{
					printf("failure  \n");
				}
			}
			else
			{
				printf("is normal or limit \n");
				ret = send(fd, &RecvInfo, sizeof(RecvInfo), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success 11 \n");
				}
				continue;
			}
		}
		else if(strcmp(RecvInfo.buf, "sendfile") == 0) //发送文件请求
		{
			int tofd;
			printf("recv flag   : %s \n", RecvInfo.flag);
			printf("recv name   : %s \n", RecvInfo.name);
			printf("recv id     : %s \n", RecvInfo.id);
			printf("recv passwd : %s \n", RecvInfo.passwd);
			printf("recv buf    : %s \n", RecvInfo.buf);
			printf("recv toname : %s \n", RecvInfo.toname);
			printf("recv mesg   : %s \n", RecvInfo.mesg);
			printf("recv rmname : %s \n", RecvInfo.rmname);
			printf("recv group  : %s \n", RecvInfo.group);
			printf("recv level  : %s \n", RecvInfo.level);
			printf("recv size   : %d \n", RecvInfo.size);

			LinkShow(first);

			//在链表中找到需要发送用户的文件描述符
			tofd = GetFd(first, RecvInfo.toname);
			if(tofd != FAILURE)
			{
				printf("get fd success : %d \n", tofd);
				ret = send(tofd, &RecvInfo, sizeof(RecvInfo), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}
			}
			//没有找到用户说明客户端发送的用户名错误，返回给客户端
			else
			{
				strcpy(RecvInfo.buf, "filefalse");
				//tofd = GetFd(first, RecvInfo.name);
				ret = send(fd, &RecvInfo, sizeof(RecvInfo), 0);
				if(ret == -1)
				{
					perror("send");
				}
				else
				{
					printf("send success \n");
				}
				printf("failure \n");
			}
		}
		else 
		{
			
		}


	}
}

/*
作者：周海阳
日期：2018.9.6
函数名：int DataLevel(Info1 ChangeData, char *level)
描述：改变成员在数据库中的权限
参数：Info1 ChangeData：接收到的结构体
char *level：用户等级（limit admin normal）
返回值：SUCCESS：成功
*/
int DataLevel(Info1 ChangeData, char *level)
{
	char sql[1024] = {0};

	sprintf(sql, "update Users set level = '%s' where name = '%s';", level, ChangeData.name);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if(ret != SQLITE_OK)
	{
		printf("sqlite3_exec error : %s \n",sqlite3_errmsg(ppdb));
		exit(1);
	}
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int SendLogin(Node *l, char *name, char *buf, int tofd)
描述：发送上线消息给其他在线用户，遍历链表中其他所有在线用户，挨个发送
参数：Node *l:链表名称
char *name：发送的用户名称
char *buf：需要发送的内容 
int tofd：发送的文件描述符号
返回值：SUCCESS：成功
FAILURE：失败
*/
int SendLogin(Node *l, char *name, char *buf, int tofd)
{
	Log templogin;
	strcpy(templogin.buf, buf);

	if(l == NULL) 
	{
		return FAILURE;
	}

	if(buf == NULL) 
	{
		return FAILURE;
	}
	
	if(name == NULL) 
	{
		return FAILURE;
	}

	Node *p = l;

	printf("in func login  \n");

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0) 
		{
			strcpy(templogin.level, p->info->level);
			ret = send(tofd, &templogin, sizeof(templogin), 0);
			if(ret == -1)
			{
				printf("sned false \n");
			}
			else
			{
				printf("send success \n");
			}
		}
	}
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int CheckAdmin(Node *l, char *name)
描述：查看用户是否为admin（管理员）
参数：Node *l:链表名称
char *name：用户名称
返回值：SUCCESS，FAILURE：不为管理员
TRUE：为管理员
*/
int CheckAdmin(Node *l, char *name)
{
	if(l == NULL) 
	{
		return FAILURE;
	}

	if(name == NULL) 
	{
		return FAILURE;
	}

	Node *p = l;

	printf("in checknn \n");

	while(p->next != NULL)
	{
		p = p->next;
		if((strcmp(p->info->name, name) == 0) && (strcmp(p->info->level, "admin") == 0))
		{
			printf("name is %s \n", p->info->name);
			printf("level is %s \n", p->info->level);
			return TRUE;
		}
		else
		{
			printf("name is %s \n", p->info->name);
			printf("level is %s \n", p->info->level);
		}
	/*	if(strcmp(p->info->name, name) == 0) 
		{
			if(strcmp(p->info->level, "admin") == 0)
			{
				return TRUE;
			}
			else
			{
				return SUCCESS;
			}
		}*/
	}
	
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int CheckNormal(Node *l, char *name)
描述：查看用户是否为normal（普通用户）
参数：Node *l:链表名称
char *name：用户名称
返回值：SUCCESS，FAILURE：不为普通用户
TRUE：为普通用户
*/
int CheckNormal(Node *l, char *name)
{
	if(l == NULL) 
	{
		return FAILURE;
	}

	if(name == NULL) 
	{
		return FAILURE;
	}

	Node *p = l;

	printf("in check normal \n");
	printf("name is %s \n", name);

	while(p->next != NULL)
	{
		p = p->next;
		if((strcmp(p->info->name, name) == 0) && (strcmp(p->info->level, "normal") == 0))
		{
			printf("name is %s \n", p->info->name);
			printf("level is %s \n", p->info->level);
			return TRUE;
		}
		else
		{
			printf("name is %s \n", p->info->name);
			printf("level is %s \n", p->info->level);
			//return SUCCESS;
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int ChangeLimit(Node *l, char *name)
描述：改变链表中用户的权限，改为limit（被限制的权限，禁言）
参数：Node *l:链表名称
char *name：用户名称
返回值：SUCCESS：成功
FAILURE：失败
*/
int ChangeLimit(Node *l, char *name)
{
	if(l == NULL) 
	{
		return FAILURE;
	}

	if(name == NULL) 
	{
		return FAILURE;
	}

	Node *p = l;

	printf("in change \n");

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0)  
		{
			if(strcmp(p->info->level, "normal") == 0)
			{
				strcpy(p->info->level, "limit");
				return TRUE;
			}
			else
			{
				return SUCCESS;
			}
		}
	}

	return FAILURE;
}
/*
作者：周海阳
日期：2018.9.6
函数名：int ChangeLevel(Node *l, char *name, char *level)
描述：改变链表中用户的权限
参数：Node *l:链表名称
char *name：用户名称
char *level：用户等级
返回值：SUCCESS：成功
FAILURE：失败
*/
int ChangeLevel(Node *l, char *name, char *level)
{
	if(l == NULL) 
	{
		return FAILURE;
	}

	if(name == NULL) 
	{
		return FAILURE;
	}

	Node *p = l;

	printf("in change \n");

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0) 
		{
			strcpy(p->info->level, level);
		}
	}

	return SUCCESS;
}


/*
作者：周海阳
日期：2018.9.6
函数名：int SendQuit(Node *l, char *name)
描述：发送下线消息给其他在线用户
参数：Node *l：链表名称
char *name：下线成员名称
返回值：SUCCESS：成功
FAILURE：失败
*/
int SendQuit(Node *l, char *name)
{
	printf("send quit \n");
	Info1 TempInfo;
	strcpy(TempInfo.buf, "quitsend");
	strcpy(TempInfo.name, name);
	if(l == NULL) 
	{
		printf("l null \n");
		return FAILURE;
	}
	
	Node *p = l;
	int tofd;
	while(p->next != NULL)  //遍历链表
	{
		p = p->next;
		//匹配到状态位为login并且昵称不为下线用户的成员
		if((strcmp(p->info->flag, "login") == 0) && (strcmp(p->info->name, name) != 0))
		{
			tofd = GetFd(first, p->info->name);
			ret = send(tofd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				perror("send");
			}
			else
			{
				printf("success \n");
			}
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int SendWelcome(Node *l, char *name)
描述：发送上线消息给其他在线用户
参数：Node *l：链表名称
char *name：上线成员名称
返回值：SUCCESS：成功
FAILURE：失败
*/
int SendWelcome(Node *l, char *name)
{
	Info1 TempInfo;
	strcpy(TempInfo.buf, "welcome");
	strcpy(TempInfo.name, name);
	if(l == NULL) 
	{
		printf("l null \n");
		return FAILURE;
	}
	
	Node *p = l;
	int tofd;
	while(p->next != NULL)
	{
		p = p->next;
		//匹配到状态位为login并且昵称不为下线用户的成员
		if((strcmp(p->info->flag, "login") == 0) && (strcmp(p->info->name, name) != 0))
		{
			tofd = GetFd(first, p->info->name);
			ret = send(tofd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				perror("send");
			}
			else
			{
				printf("success \n");
			}
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
描述：改变聊天室链表的状态flag（online or quit）
函数名：int GroupLinkFlag(GNode *l, char *name, char *flag)
参数：GNode *l：链表名称
	char *name：成员名称
	char *flag：online在线 还是 quit退出
返回值：SUCCESS：成功
		FAILURE：失败
*/
int GroupLinkFlag(GNode *l, char *name, char *flag)
{
	if(l == NULL) 
	{
		return FAILURE;
	}

	if(flag == NULL) 
	{
		return FAILURE;
	}
	
	if(name == NULL) 
	{
		return FAILURE;
	}

	GNode *p = l;

	printf("in flag \n");
	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0)
		{
			strcpy(p->info->flag, flag);
			//return SUCCESS;
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int GroupLinkRoot(GNode *l, char *name, char *root)
描述：改变聊天室链表成员的权限root（admin or normal）
参数：GNode *l：链表名称
	char *name：成员名称
	char *root：admin：管理员 normal普通成员
返回值：SUCCESS：成功
		FAILURE：失败
*/
int GroupLinkRoot(GNode *l, char *name, char *root)
{
	if((l == NULL) || (root == NULL))
	{
		return FAILURE;
	}
	GNode *p = l;

	printf("in root \n");
	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0)
		{
			strcpy(p->info->level, root);
			return SUCCESS;
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名:int RoomKick(GNode *l, char *name, int tofd)
描述：踢出聊天室的成员
参数：GNode *l：链表名称
	char *name：踢出成员名称
	int tofd：被提出成员的文件描述符用来发送
返回值：SUCCESS：成功
		FAILURE：失败
*/

int RoomKick(GNode *l, char *name, int tofd)
{
	Info1 TempInfo;
	GNode *p = l;

	printf("name is %s \n", name);
	printf("tofd is %d \n", tofd);
	printf("in kick \n");
	int ToFd;
	if(l == NULL || name == NULL)
	{
		printf("null \n");
		return FAILURE;
	}
 
	while(p->next != NULL)
	{
		printf("in while \n");
		p = p->next;
		if(strcmp(p->info->name, name) == 0)
		{
			printf("in if \n");
			strcpy(p->info->flag, "quit");
			printf("in if  2\n");
			strcpy(TempInfo.buf, "roomquit");
			ret = send(tofd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send failure \n");
			}
			else
			{
				printf("send success \n");
			}
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int CkeckAdmin(GNode *l, char *name)
描述：检查成员是否为管理员
参数：GNode *l：链表名称
	char *name：检查成员名称
返回值：SUCCESS：成功
		FAILURE：失败
		TURE：是管理员	
*/
int CkeckAdmin(GNode *l, char *name)
{
	if(l == NULL || name == NULL)
	{
		return FAILURE;
	}

	GNode *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if((strcmp(p->info->name, name) == 0 )&& (strcmp(p->info->level, "admin") == 0))
		{
			return TRUE;		
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int RoomChat(GNode *l, char *flag, char *name, char *group, char *mesg)
描述：和聊天室的成员进行聊天（群聊）
参数：GNode *l：链表名称
	char *name：发送消息成员的昵称
	char *flag：online在线，遍历链表找出在线的成员
	char *group：聊天室的名称
	char *mesg：发送的消息内容
返回值：SUCCESS：成功
		FAILURE：失败
*/
int RoomChat(GNode *l, char *flag, char *name, char *group, char *mesg)
{
	if(l == NULL || flag == NULL || NULL == mesg || group == NULL)
	{
		return FAILURE;
	}

	GNode *p = l;
	Info1 TempInfo;
	int TempFd;
	
	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->flag, flag) == 0)
		{
			strcpy(TempInfo.buf, "roomchat");
			strcpy(TempInfo.name, name);
			strcpy(TempInfo.group, group);
			strcpy(TempInfo.mesg, mesg);
			TempFd = p->info->fd;
			ret = send(TempFd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send failure \n");
			}
			else
			{
				printf("send success \n");
			}

		}
	}
}

/*
作者：周海阳
日期：2018.9.6
描述：和在线的成员进行聊天（群聊）
函数名：int GroupChat(Node *l, char *flag, char *name, char *mesg)
参数：Node *l：链表名称
	char *flag：online在线，遍历链表找出在线的成员
	char *name：发送消息成员的昵称
	char *mesg：发送的消息内容
返回值：SUCCESS：成功
		FAILURE：失败
*/
int GroupChat(Node *l, char *flag, char *name, char *mesg)
{
	if(l == NULL || flag == NULL || NULL == mesg)
	{
		return FAILURE;
	}

	Node *p = l;
	Info1 TempInfo;
	int TempFd;
	
	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->flag, flag) == 0)
		{
			strcpy(TempInfo.buf, "group");
			strcpy(TempInfo.name, name);
			strcpy(TempInfo.mesg, mesg);
			TempFd = p->info->fd;
			ret = send(TempFd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send failure \n");
			}
			else
			{
				printf("send success \n");
			}
		
		}
	
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int GetFd(Node *l, char *name)
描述：遍历链表成员获取其文件描述符
参数：Node *l：链表名称
	char *name：需要查找文件描述符成员的名称
返回值：fd：成员的文件描述符
		FAILURE：失败
*/

int GetFd(Node *l, char *name)
{
	if(l == NULL || name == NULL)
	{
		return FAILURE;
	}

	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if((strcmp(p->info->name, name) == 0) && (strcmp(p->info->flag, "login") == 0))
		{
			return p->info->fd;
		}
	}
	return FAILURE;

}

/*
作者：周海阳
日期：2018.9.6
函数名：int RoomCheckon(GNode *l, char *flag, int fd)
描述：遍历链表查询聊天室在线的成员名称，并发送到发起查找请求的成员服务端
参数：GNode *l：链表名称
	char *flag：online在线，遍历链表找出在线的成员
	fd：成员的文件描述符，发送给客户端的时候需要
返回值：SUCCESS：成功
		FAILURE：失败
*/
int RoomCheckon(GNode *l, char *flag, int fd)
{
	Info1 TempInfo;
	if(NULL == l)
	{
		return FAILURE;
	}

	GNode *p = l;
	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->flag, flag) == 0)
		{	
			strcpy(TempInfo.buf, "roomcheckon");
			strcpy(TempInfo.name, p->info->name);
			printf("online name : %s \n", TempInfo.name);
			ret = send(fd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send failure \n");
			}
			else
			{
				printf("send success \n");
			}
		
		}

	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int CheckOn(Node *l, char *flag, int fd)
描述：遍历链表查询整个在线的成员名称，并发送到发起查找请求的成员服务端
参数：Node *l：链表名称
	char *flag：online在线，遍历链表找出在线的成员
	fd：成员的文件描述符，发送给客户端的时候需要
返回值：SUCCESS：成功
		FAILURE：失败
*/
int CheckOn(Node *l, char *flag, int fd)
{
	Info1 TempInfo;
	if(NULL == l)
	{
		return FAILURE;
	}
	
	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->flag, flag) == 0)
		{
			strcpy(TempInfo.buf, "checkon");
			strcpy(TempInfo.flag, p->info->flag);
			strcpy(TempInfo.name, p->info->name);
			strcpy(TempInfo.id, p->info->id);
			strcpy(TempInfo.passwd, p->info->passwd);
			printf("login name : %s \n", TempInfo.name);
			ret = send(fd, &TempInfo, sizeof(TempInfo), 0);
			if(ret == -1)
			{
				printf("send failure \n");
			}
			else
			{
				printf("send success \n");
			}
		}
	}
	
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int LinkChangeFd(Node *l, Info1 *ChangeInfo, int fd)
描述：遍历链表改变成员的文件描述符
参数：Node *l：链表名称
	Info1 *ChangeInfo：结构体，获取里面的name
	fd：成员的文件描述符
返回值：SUCCESS：成功
		FAILURE：失败
*/
int LinkChangeFd(Node *l, Info1 *ChangeInfo, int fd)
{
	Node *p = l;

	if((l == NULL) || (ChangeInfo == NULL) )
	{
		return FAILURE;
	}

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, ChangeInfo->name) == 0)
		{
			p->info->fd = fd;
			return SUCCESS;
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int LinkChange(Node *l, Info1 *ChangeInfo, char *flag)
描述：遍历链表改变成员的在线状态
参数：Node *l：链表名称
	Info1 *ChangeInfo：结构体，获取里面的name
	char *flag：register 已注册 login 已登录 quit 退出
返回值：SUCCESS：成功
		FAILURE：失败
*/
int LinkChange(Node *l, Info1 *ChangeInfo, char *flag)
{
	Node *p = l;

	if((l == NULL) || (ChangeInfo == NULL) || (flag == NULL))
	{
		return FAILURE;
	}


	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, ChangeInfo->name) == 0)
		{
			strcpy(p->info->flag, flag);
			return SUCCESS;
		}
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：DataChangeFd(Info1 ChangeData, int fd)
描述：更新数据库里面成员的文件描述符
参数：Info1 ChangeData：结构体
		int fd：文件描述符
返回值：
*/
int DataChangeFd(Info1 ChangeData, int fd)
{
	char sql[1024] = {0};

	sprintf(sql, "update Users set fd = '%d' where name = '%s';", fd, ChangeData.name);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if(ret != SQLITE_OK)
	{
		printf("sqlite3_exec error : %s \n",sqlite3_errmsg(ppdb));
		exit(1);
	}
}

/*
作者：周海阳
日期：2018.9.6
函数名：int DataChange(Info1 ChangeData, char *flag)
描述：更新数据库里面成员的登录状态
参数：Info1 ChangeData：结构体
		char *flag：register 已注册 login 已登录 quit 退出
返回值：
*/
int DataChange(Info1 ChangeData, char *flag)
{
	char sql[1024] = {0};

	sprintf(sql, "update Users set flag = '%s' where name = '%s';", flag, ChangeData.name);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if(ret != SQLITE_OK)
	{
		printf("sqlite3_exec error : %s \n",sqlite3_errmsg(ppdb));
		exit(1);
	}
}


/*
作者：周海阳
日期：2018.9.6
函数名：int DataWrite(Info1 WriteInfo, int fd)
描述：创建一个数据库，存放成员的flag：在线状态, name：名称, id, passwd：密码, fd：文件描述符
参数：Info1 WriteInfo
		int fd：文件描述符
返回值：SUCCESS：成功
		FAILURE：失败
*/
int DataWrite(Info1 WriteInfo, int fd)
{
	char sql[1024] = {0};

	sprintf(sql, "create table if not exists Users (flag text, name text, id text, passwd text, fd integer, level text);");
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if(ret != SQLITE_OK)
	{
		printf("1.sqlite3_exec error : %s \n",sqlite3_errmsg(ppdb));
		return FAILURE;
	}

	sprintf(sql, "insert into Users (flag, name, id, passwd, fd, level) values('%s', '%s', '%s', '%s', '%d', '%s');"
			,WriteInfo.flag, WriteInfo.name, WriteInfo.id, WriteInfo.passwd, fd, WriteInfo.level);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if(ret != SQLITE_OK)
	{
		printf("2.sqlite3_exec error : %s \n",sqlite3_errmsg(ppdb));
		return FAILURE;
	}

	return SUCCESS;
}
	
/*
作者：周海阳
日期：2018.9.6
函数名：int DataShow()
描述：显示数据库的信息
参数：
返回值：SUCCESS：成功
		FAILURE：失败
*/
int DataShow()
{
	char sql[1024];
	sprintf(sql, "select * from Users;"); 
	ret = sqlite3_exec(ppdb, sql, show, NULL, NULL);
	if(ret != SQLITE_OK)
	{
		printf("3.sqlite3_exec error  : %s \n",sqlite3_errmsg(ppdb));
		return FAILURE;
	}
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int LinkReadData()
描述：将数据库的信息写道链表中
参数：
返回值：SUCCESS：成功
		FAILURE：失败
*/
int LinkReadData()
{
	char sql[1024] = {0};
	if(link == NULL)
	{
		return FAILURE;
	}

	sprintf(sql, "select * from Users;");
	ret = sqlite3_exec(ppdb, sql, ReadData, NULL, NULL);
	if(ret != SQLITE_OK)
	{
		printf("4.sqlite3_exec error  : %s \n",sqlite3_errmsg(ppdb));
	}
	
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int ReadData(void *para, int columnCount, char **columnValue, char **columnName)
描述：sqlite3_exec(ppdb, sql, ReadData, NULL, NULL);函数的执行函数
用来将数据库的信息写道链表中
参数：void *para, 
int columnCount, 数据库的每项数据个数
char **columnValue, 数据的值
char **columnName，数据的名称
返回值：SUCCESS：成功
		FAILURE：失败
*/
int ReadData(void *para, int columnCount, char **columnValue, char **columnName)
{
	Info1 linkInfo;
	int fd;

	strcpy(linkInfo.flag, columnValue[0]);
	strcpy(linkInfo.name, columnValue[1]);
	strcpy(linkInfo.id, columnValue[2]);
	strcpy(linkInfo.passwd, columnValue[3]);
	fd = atoi(columnValue[4]);
	strcpy(linkInfo.level, columnValue[5]);

	LinkInsert(first, &linkInfo, fd);

	return 0;
}
/*
作者：周海阳
日期：2018.9.6
函数名：int show(void *para, int columnCount, char **columnValue, char **columnName)
描述：sqlite3_exec(ppdb, sql, show, NULL, NULL);函数的执行函数
显示数据库的信息
参数：void *para, 
int columnCount, 数据库的每项数据个数
char **columnValue, 数据的值
char **columnName，数据的名称
返回值：SUCCESS：成功
		FAILURE：失败
*/
int show(void *para, int columnCount, char **columnValue, char **columnName)
{
	int i;
	for(i = 0; i < columnCount; i++)
	{
		printf("%s = %s ",columnName[i], columnValue[i]);
	}
	printf("\n");

	return 0;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int GroupLinkInit(GNode **l)
描述：初始化聊天室链表
参数：GNode **l：链表名称
返回值：SUCCESS：成功
		FAILURE：失败
*/
int GroupLinkInit(GNode **l)
{
	*l = (GNode *)malloc(sizeof(GNode));
	if(NULL == *l)
	{
		return FAILURE;
	}

	(*l)->info = NULL;
	(*l)->next = NULL;
	
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int GroupLinkInsert(GNode *l, Info1 *InsertInfo, int fd, char *root)
描述：向聊天室链表中插入成员信息
参数：GNode *l：链表名称
Info1 *InsertInfo：结构体
int fd：成员的文件描述符
char *root：admin：管理员 normal普通成员
返回值：SUCCESS：成功
		FAILURE：失败
*/
int GroupLinkInsert(GNode *l, Info1 *InsertInfo, int fd, char *root)
{
	if(l == NULL)
	{
		return FAILURE;
	}
	
	if(InsertInfo == NULL)
	{
		return FAILURE;
	}

	GNode *p = l;
	while(p->next != NULL)
	{
		p = p->next;
	}
	GNode *n = (GNode *)malloc(sizeof(GNode));
	Info3 *info = (Info3 *)malloc(sizeof(Info3));

	p->next = n;

	strcpy(info->flag, "online");
	strcpy(info->name, InsertInfo->name);
	strcpy(info->level, root);
	info->fd = fd;

	n->info = info;
	n->next = NULL;

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int GroupLinkShow(GNode *l)
描述：显示聊天室链表的成员信息
参数：GNode *l：链表名称
返回值：SUCCESS：成功
		FAILURE：失败
*/
int GroupLinkShow(GNode *l)
{
	printf("in link show \n");
	if(NULL == l)
	{
		return FAILURE;
	}
	
	GNode *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		printf("flag : %s name: %s fd: %d level: %s\n", p->info->flag, p->info->name, p->info->fd, p->info->level);
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int LinkInit(Node **l)
描述：初始化软件链表
参数：GNode **l：链表名称
返回值：SUCCESS：成功
		FAILURE：失败
*/
int LinkInit(Node **l)
{
	*l = (Node *)malloc(sizeof(Node));
	if(NULL == *l)
	{
		return FAILURE;
	}

	(*l)->info = NULL;
	(*l)->next = NULL;
	
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int LinkInsert(Node *l, Info1 *InsertInfo, int fd)
描述：向软件链表中插入成员信息
参数：Node *l：链表名称
Info1 *InsertInfo：结构体
int fd：文件描述符
返回值：SUCCESS：成功
		FAILURE：失败
*/
int LinkInsert(Node *l, Info1 *InsertInfo, int fd)
{
	if(l == NULL)
	{
		return FAILURE;
	}
	
	if(InsertInfo == NULL)
	{
		return FAILURE;
	}

	Node *p = l;
	while(p->next != NULL)
	{
		p = p->next;
	}
	Node *n = (Node *)malloc(sizeof(Node));
	Info2 *info = (Info2 *)malloc(sizeof(Info2));

	p->next = n;

	strcpy(info->flag, InsertInfo->flag);
	strcpy(info->name, InsertInfo->name);
	strcpy(info->id, InsertInfo->id);
	strcpy(info->passwd, InsertInfo->passwd);
	strcpy(info->level, InsertInfo->level);
	info->fd = fd;

	n->info = info;
	n->next = NULL;

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int LinkShow(Node *l)
描述：向软件链表中插入成员信息
参数：Node *l：链表名称
返回值：SUCCESS：成功
		FAILURE：失败
*/
int LinkShow(Node *l)
{
	printf("in link show \n");
	if(NULL == l)
	{
		return FAILURE;
	}
	
	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		printf("link info :%s %s %s %s %s fd:%d \n", p->info->flag, p->info->name, p->info->id, p->info->passwd, 
			p->info->level,	p->info->fd);
	}

	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int RepetitionName(Node *l, char *name)
描述：检查链表中是否有与name重复的元素
参数：Node *l：链表名称
char *name：检查重复的成员名称
返回值：SUCCESS：成功
		FAILURE：失败
*/
int RepetitionName(Node *l, char *name)
{
	if(l == NULL || name == NULL)
	{
		return FAILURE;
	}

	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0)
		{
			return TRUE;
		}
	}
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int RepetitionId(Node *l, char *id)
描述：检查链表中是否有与id重复的元素
参数：Node *l：链表名称
char *id：检查重复的id
返回值：SUCCESS：成功
		FAILURE：失败
*/
int RepetitionId(Node *l, char *id)
{
	if(l == NULL || id == NULL)
	{
		return FAILURE;
	}

	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->id, id) == 0)
		{
			return TRUE;
		}
	}
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int RepetitionPasswd(Node *l, char *passwd)
描述：检查链表中是否有与passwd重复的元素
参数：Node *l：链表名称
char *passwd：检查重复的passwd
返回值：SUCCESS：成功
		FAILURE：失败
*/
int RepetitionPasswd(Node *l, char *passwd)
{
	if(l == NULL || passwd == NULL)
	{
		return FAILURE;
	}

	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->passwd, passwd) == 0)
		{
			return TRUE;
		}
	}
	return SUCCESS;
}

/*
作者：周海阳
日期：2018.9.6
函数名：int LoginCheck(Node *l, char *name, char *id, char *passwd)
描述：登陆时检查成员信息是否与链表中的信息相同
参数：Node *l：链表名称
char *name：成员名称
char *id：检查重复的id
char *passwd：检查重复的passwd
返回值：SUCCESS：成功
		FAILURE：失败
		TRUE：
*/
int LoginCheck(Node *l, char *name, char *id, char *passwd)
{
	if(l == NULL || passwd == NULL || id == NULL || passwd == NULL)
	{
		return FAILURE;
	}

	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0) 
		{
			if(strcmp(p->info->id, id) == 0)
			{
				if(strcmp(p->info->passwd, passwd) == 0)
				{
					return TRUE;
				}
			}
		}
	}

	return SUCCESS;
}
/*
作者：周海阳
日期：2018.9.6
函数名：int LoginCheckon(Node *l, char *name)
描述：登陆时检查成员是否在之前已经登陆过
参数：Node *l：链表名称
char *name：成员昵称
返回值：SUCCESS：成功
		FAILURE：失败
		TRUE：
*/

int LoginCheckon(Node *l, char *name)
{
	if(l == NULL || name == NULL)
	{
		return FAILURE;
	}

	Node *p = l;

	while(p->next != NULL)
	{
		p = p->next;
		if(strcmp(p->info->name, name) == 0) 
		{
			if( (strcmp(p->info->flag, "register") == 0) || (strcmp(p->info->flag, "quit") == 0) )
			{
					return TRUE;
			}
		}
	}

	return SUCCESS;
}








