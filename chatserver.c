#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>

#define QLEN 32
#define BUFSIZE 256

const int PORT = 3344;
int ssocks[QLEN] = {NULL};

void recvMsg(int fd);
void broadcastMsg(int, char *,int);
int errexit(const char *format, ...);
int passiveTCP(int port, int qlen);

int main(int argc, char *argv[])
{
    pthread_t th[QLEN];
    int conncount = 0;
    int port = PORT;
    struct sockaddr_in fsin;
    unsigned int alen;
    int msock;
    // int ssocks
    switch (argc)
    {
    case 1:
        break;
    case 2:
        port = atoi(argv[1]);
    default:
        errexit("usage:ChatServer[port]\n");
    }

    //获取主套接字
    msock = passiveTCP(port, QLEN);

    while (1)
    {
        alen = sizeof(fsin);
        ssocks[conncount++] = accept(msock, (struct sockaddr *)&fsin, &alen);
        if (ssocks[conncount - 1] < 0)
        {
            if (errno == EINTR)
                continue;
            errexit("accept: %s\n", strerror(errno));
        }
        printf("服务器与客户端建立链接...\n\n");
        if (pthread_create(&th[conncount-1], NULL, (void *(*)(void *))recvMsg,
                           (void *)ssocks[conncount - 1]) < 0)
            errexit("phread_create:%s\n", strerror(errno));
    }
}

/*
*   接收客户端信息并转发
*/
void recvMsg(int fd)
{
    int cc;
    char buf[BUFSIZ + 1];
    printf("进入转发线程\n");
    //显示欢迎界面
    char str[BUFSIZE+1] =
        "-------------------------------------------------------------\n------------------------欢迎加入聊天组-----------------------\n-------------------------------------------------------------\n\n";
    str[BUFSIZE]='\0';
    printf("fd: %d\n",fd);
    if (write(fd, str, sizeof buf) < 0)
        errexit("发送欢迎界面失败！%s\n", strerror(errno));
    strcpy(str,"---有新成员加入---\n\n");
    broadcastMsg(fd, str, sizeof str);
    while (1)
    {
        memset(buf,'\0',strlen(buf));
        //接收改客户端信息并转发给其他客户端
        while (cc = read(fd, buf, sizeof buf))
        {
            if (cc < 0)
                errexit("recv:%s\n", strerror(errno));
            printf("%s",buf);
            broadcastMsg(fd,buf,cc);
        }
        
    }
    shutdown(fd,2);
}

//广播消息
void broadcastMsg(int fd, char *buf,int cc)
{
    for (int i = 0; i < QLEN; i++)
    {
        //printf("准备转发...\n");
        if (ssocks[i] != NULL && ssocks[i]!=fd )
        {
            if (write(ssocks[i], buf, cc) < 0){}
                //errexit("广播消息失败！%s\n", strerror(errno));
        }
    }
    buf = "广播完成\n";
    fputs(buf,stdout);
}
