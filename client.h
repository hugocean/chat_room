#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 		8888
#define FAILURE		10000
#define SUCCESS		10001
#define TRUE		10002
#define FALSE		10003


struct sockaddr_in server_addr;

struct login
{
	char buf[10];
	char level[16];
};
typedef struct login Log;
//这个结构体成员用来和服务器之间进行通信用的
struct user_info
{
	char flag[16];   //login register quit
	char buf[16];
	char name[32];
	char id[32];
	char passwd[32];
	char toname[32];
	char mesg[64];
	char rmname[32];
	char group[32];
	char level[16];
	char lmname[16];
	ssize_t size;
};
typedef struct user_info Info1;


void *RecvMsg(void *arg);
void *SendMsg(void *arg);
void StopClient(int num);
int ClientInit();
int UserReg(int sockfd);
int UserLogin(int sockfd);
int Quit(int sockfd);
int IntoMyZone(int sockfd);
int PrivateChat(int sockfd);
int ChatRecords();
int EnterGroup(int sockfd);
int RoomAdmin(int sockfd);
int RoomKick(int sockfd, char *name);
int RoomQuit(int sockfd);
int RoomCheckon(int sockfd);
int RoomChat(int sockfd);
int CreatGroup(int sockfd);
int GroupChat(int sockfd);
int PrivateChat(int sockfd);
int CheckFriend(int sockfd);



#endif











