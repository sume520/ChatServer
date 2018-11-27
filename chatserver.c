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
#define BUFSIZE 4096

const int PORT = 3344;
int msock;
int ssocks[QLEN] = {NULL};

void recvMsg(int fd);
void broadcastMsg(int, char *);
int errexit(const char *format, ...);
int passiveTCP(int port, int qlen);

int main(int argc, char *argv[])
{
    pthread_t th[QLEN];
    int conncount = 0;
    int port = PORT;
    struct sockaddr_in fsin;
    unsigned int alen;
    //int msock;
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
        char *str="Hello World!\n";
        if (write(ssocks[conncount-1], str, sizeof str) < 0)
        errexit("发送失败！%s\n", strerror(errno));
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
    // char *str =
    //     "-------------------------------------------------------------\n---------------------欢迎加入聊天组--------------------\n--------------------------------------------------------------\n";
    char *str="欢迎加入聊天组\n";
    printf("fd: %d\n",fd);
    if (write(fd, str, sizeof str) < 0)
        errexit("发送欢迎界面失败！%s\n", strerror(errno));
     str = "有新成员加入聊天组\n\0";
     broadcastMsg(fd, str);
    while (1)
    {
        memset(buf,NULL,sizeof(buf));
        //接收改客户端信息并转发给其他客户端
        while (cc = read(fd, buf, sizeof buf))
        {
            fflush(stdin);
            printf("正在读取...\n");
            if (cc < 0)
                errexit("recv:%s\n", strerror(errno));
            printf("%s",buf);
            broadcastMsg(fd,buf);
            fflush(stdout);
        }
        
    }
    close(fd);
}

//广播消息
void broadcastMsg(int fd, char *buf)
{
    printf("不转发的fd: %d\n",fd);
    char *buff=buf;
    for (int i = 0; i < QLEN; i++)
    {
        //printf("准备转发...\n");
        if (ssocks[i] != NULL && ssocks[i] != fd )
        {
            printf("转发的fd：%d\n",ssocks[i]);
            if (write(ssocks[i], buff, sizeof buff) < 0)
                errexit("广播消息失败！%s\n", strerror(errno));
        }
        fflush(stdout);
    }
    buff = "广播完成\n";
    fputs(buff,stdout);
}
