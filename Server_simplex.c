/* tcp 服务器（单工）*/
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#define SERV_PORT 5050          // 注意地区的一致性
#define MAX 1000
int main() {
    int listen_fd, conn_fd, n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;       //unsigned int
    char msg[MAX];
    listen_fd = socket(AF_INET,SOCK_STREAM,0);
    //配置网络地址
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
/*
将主机的无符号长整形数转换成网络字节顺序
#include <arpa/inet.h>　　
uint32_t htonl(uint32_t hostlong);
hostlong：主机字节顺序表达的32位数　　
注释：
  　本函数将一个32位数从主机字节顺序转换成网络字节顺序
返回值：　
    htonl()返回一个网络字节顺序的值
*/
    bind(listen_fd,(struct sockaddr *)&servaddr,sizeof(servaddr));      //将网络地址绑定套接字
/*
bind函数
作用：将一个socket与一个地址结构绑定，使其与指定的端口号和 IP 地址相关联
原型：int bind(int sockfd,struct sockaddr * my_addr,int addrlen);
参数：
sockfd：socket文件描述符
sockaddr：地址结构指针，结构体类型和地址的类型相关
addrlen：sizeof（struct sockaddr），地址结构体的长度
返回值：成功则返回 0，失败返回-1
*/
    listen(listen_fd,100);              //服务器监听
/*
listen 函数
作用：listen函数将一个socket注册为被动socket，用来监听等待其他socket的主动connect。一个成功connect的socket无法再注册为被动socket
原型：int listen(int sockfd,int backlog);
参数：sockfd为套接字描述符，backlog可以理解为该socket同时能处理的最大连接要求，通常为 5 或者 10，最大值可设至 128
返回值：成功则返回 0，失败返回-1
*/
    while(1) {
        clilen = sizeof(cliaddr);
        conn_fd = accept(listen_fd,(struct sockaddr *)&cliaddr,&clilen); //每有一个客户端生成一个连接套接字
/*
在监听socket上接受一个调用了connect的主动socket的连接，如果不存在接入连接则阻塞等待连接
调用accept成功后会创建一个新的socket与发起连接的socekt建立连接并进行后续通信
*/
        while(1) {
            n = recvfrom(conn_fd,msg,MAX,0,(struct sockaddr *)&cliaddr,&clilen);    //接受客户端的信息
            if(n == 0) {
                printf("client offline!\n");
                break;
            }
            sendto(conn_fd,msg,n,0,(struct sockaddr *)&cliaddr,clilen);             //向客户端发送信息
            printf("-------------------------------\n");
            msg[n] = '\0';
            printf("received the following:\n");
            printf("%s",msg);
            printf("-------------------------------\n");
        }
        close(conn_fd);
    }
    close(listen_fd);
    return 0;
}
/*
linux网络通信（服务端(中间人)）
1.socket生成套接字文件用来在一个端口的两个对象之间传输数据
2.初始化包含需传输数据的结构体
3.设置数据结构体中的地址族，internet地址，端口号
（前三步与客户端相同）
4.将网络地址结构体绑定到套接字文件上（绑定设置端口号和参数ip地址）
5.服务器listen将一个socket注册为被动socket，用来监听等待其他socket的主动connect（监听连接信号）
6.在listen注册的被动socket上又调用了一个主动的socket连接，当连接出现时立刻调用connet创建一个新的socket与发起连接的原socket建立连接从而通信
7.用recvfrom接收一个数据包socket传来的数据包，并获得发送方的地址
8.将获取的数据包发送给另一个客户端
(记住要先开服务端)
9.关闭新建套接字和接受的套接字
*/
/*
tcp-服务器

1. 创建一个socket，用函数socket()

2. 绑定IP地址、端口等信息到socket上，用函数bind()

3.设置允许的最大连接数，用函数listen()

4.接收客户端上来的连接，用函数accept()

5.收发数据，用函数send()和recv()，或者read()和write()

6.关闭网络连接
*/
