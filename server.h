#ifndef __SERVER_H
#define __SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sqlite3.h>

#define PORT 		8888
#define FAILURE		10000
#define SUCCESS		10001
#define TRUE		10002
#define FALSE		10003
#define SIZE 		1000



struct sockaddr_in server_addr;
struct sockaddr_in client_addr;


struct login
{
	char buf[10];
	char level[16];
};
typedef struct login Log;



//结构体Info1用来在服务器和客户端之通信
struct user_info
{
	char flag[16];   //login已登录 register已注册 quit已退出
	char buf[16]; 	//判断客户端来的请求	
	char name[32];	//用户的昵称
	char id[32];	//用户的id
	char passwd[32];//用户的登陆密码
	char toname[32];//用户需要私聊的成员昵称
	char mesg[64];	//用户需要发送的消息
	char rmname[32];//用户需要在聊天室踢出的成员名称
	char group[32];	//用户想创建聊天室的名称
	char level[16];  //用户权限limit normal admin
	char lmname[16]; //如果你是管理员你想禁言成员的名称
	ssize_t size;
};
typedef struct user_info Info1;

//结构体Info2用来写到链表和数据库的信息
struct link_info
{
	char flag[16];   //login已登录 register已注册 quit已退出
	char name[32];	//用户的昵称
	char id[32];	//用户的id
	char passwd[32];//用户的登陆密码
	char level[16]; //用户权限limit normal admin
	int	 fd;		//客户端的文件描述符
};
typedef struct link_info Info2;

//结构体Node软件链表的结点
struct link_node
{
	struct link_info *info; //指向结构体Info2
	struct link_node *next;	//指向下一个结点的地址
};
typedef struct link_node Node;

//结构体Info3用来写聊天室成员的信息
struct group_info
{
	char flag[16];  //聊天室成员的在线状态 online：在线 quit：下线
	char name[32];	//聊天室成员的名称
	char level[16];	//聊天室成员的权限
	int	 fd;		//聊天室成员的文件描述符
};
typedef struct group_info Info3;

//结构体GNode聊天室链表的结点
struct group_node
{
	struct group_info *info;
	struct group_node *next;
};
typedef struct group_node GNode;

sqlite3 *ppdb;

int ServerInit();
int ServerAccept(int sockFd);
void *ClientHandler(void *arg);
int LinkChange(Node *l, Info1 *ChangeInfo, char *flag);
int DataChange(Info1 ChangeData, char *flag);
int DataWrite(Info1 WriteInfo, int fd);
int DataShow();
int LinkReadData();
int ReadData(void *para, int columnCount, char **columnValue, char **columnName);
int show(void *para, int columnCount, char **columnValue, char **columnName);
int LinkInit(Node **l);
int LinkShow(Node *l);
int LinkInsert(Node *l, Info1 *InsertInfo, int fd);
int TraversFriend(Node *l, Info1 CheckInfo, char *flag);
int RepetitionName(Node *l, char *name);
int RepetitionId(Node *l, char *id);
int RepetitionPasswd(Node *l, char *passwd);
int LoginCheckon(Node *l, char *name);
int LoginCheck(Node *l, char *name, char *id, char *passwd);
int GroupLinkInit(GNode **l);
int GroupLinkShow(GNode *l);
int GroupLinkInsert(GNode *l, Info1 *InsertInfo, int fd, char *root);
int CheckOn(Node *l, char *flag, int fd);
int RoomCheckon(GNode *l, char *flag, int fd);
int GetFd(Node *l, char *name);
int GroupChat(Node *l, char *flag, char *name, char *mesg);
int RoomChat(GNode *l, char *flag, char *name, char *group, char *mesg);
int CkeckAdmin(GNode *l, char *name);
int RoomKick(GNode *l, char *name, int tofd);
int GroupLinkRoot(GNode *l, char *name, char *root);
int GroupLinkFlag(GNode *l, char *name, char *flag);
int SendWelcome(Node *l, char *name);
int SendQuit(Node *l, char *name);




#endif




