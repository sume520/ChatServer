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

const int PORT=3344;
int msock;
int ssock[QLEN];

int recvMsg(int fd);
void broadcastMsg(char*);
int errexit(const char* format,...);
int passiveTCP(int port,int qlen);

int main(int argc,char *argv[])
{
    pthread_t	th;
    int conncount=0;
    int port=PORT;
    struct sockaddr_in fsin;
    unsigned int alen;
    // int msock;
    // int ssock;
    switch(argc){
    case 1:
        break;
    case 2:
        port=atoi(argv[1]);
    default:
        errexit("usage:ChatServer[port]\n");
    }

    //获取主套接字
    msock=passiveTCP(port,QLEN);

    while(1){
        alen=sizeof(fsin);
        ssock[conncount++]=accept(msock,(struct sockaddr*)&fsin,&alen);
        if(ssock[conncount-1]<0){
            if (errno == EINTR)
				continue;
			errexit("accept: %s\n", strerror(errno));
        }
        if(pthread_create(&th,0,(void*(*)(void*))recvMsg,
            (void*)(long)ssock[conncount-1])<0)
            errexit("phread_create:%s\n",strerror(errno));
    }
}

/*
*   接收客户端信息并转发
*/
int recvMsg(int fd){
    int cc;
    char buf[BUFSIZ];

    while (cc = read(fd, buf, sizeof buf)){
        if(cc<0)
            errexit("recv:%s\n",strerror(errno));
        broadcastMsg(buf);
    }

    return 0;
}

//广播消息
void broadcastMsg(char *buf){

}
