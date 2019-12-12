#ifndef _TRANS_H_
#define _TRANS_H_

#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

//两个有用的宏定义：检查和赋值并且检测
#define CHK(eval) if(eval < 0){perror("eval"); exit(-1);}
#define CHK2(res, eval) if((res = eval) < 0){perror("eval2"); exit(-1);}

#define TRANS_MSGSIZE	1400
#define TRANS_CMSGSIZE	1522
//#define SERVERIP	"127.1.0.1"
#define SERVERIP	"192.168.1.1"
#define	SERVERPORT	22222

//设备初始化使用
#define IPQ_CMD_SET_CHID _IO(0x11, 0)
#define IPQ_CMD_ATTACH_CHID _IO(0x11, 1)

typedef struct _trans
{
    char *(* openfd)(int *fd);
    char *(* closefd)(int *fd);
    char *(* send_message)(int fd);
	char *(* recv_message)(int fd);
}trans_t;

typedef struct _msg
{
	unsigned int seq;
	unsigned int speed;
	char data[0];
}msg_t;

typedef struct _cfg
{
	int method;
	char *type;
	int bagnum;
	int delay;
  char ip[16];
  int port;
}cfg_t;

#endif
