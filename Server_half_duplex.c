# include<sys/socket.h>
# include<netinet/in.h>
# include<arpa/inet.h>
# include<signal.h>
# include<assert.h>
# include<stdio.h>
# include<unistd.h>
# include<string.h>
# include<stdlib.h>
# include<errno.h>
# define MAX 1024             

int main (int argc,char * argv[]) {             

    const char * ip = argv[1];                  // 将地址和端口号作为参数手动输入
    int port = atoi(argv[2]);                   // 将输入的端口号由字符串转换为整数类型(atoi将字符串转换为整数型)
    /*结构体定义与初始化*/
    struct sockaddr_in address;
    bzero(&address,sizeof(address));            /*初始化清零,类似于memset函数*/
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);    /*inet_pton是inet_aton的升级版，随IPV6的出现而出现*/
    // 将IP地址从字符串格式转换成网络地址格式，支持Ipv4和Ipv6.（inet_pton4代表IPV4，inet_pton6代表IPV6）
/*
函数原型： static int inet_pton(int af, const char *src,void *dst)
af: address family(协议族)，支持的协议族有下面几种：AF_INET Inetnet的Ipv4协议 / Inetnet的Ipv6协议
src:是个指针，指向保存IP地址字符串形式的字符串。
dst:指向存放网络地址的结构体的首地址
*/
    address.sin_port = htons(port);                 /*将小端字节序转换为网络字节序（大端），统一格式*/

    int sock = socket(PF_INET, SOCK_STREAM, 0);     /*创建套接字*/
    assert(sock >= 0);                              // 当条件成立时不做任何操作

    int ret = bind(sock,(struct sockaddr*)&address,sizeof(address));        /*绑定IP地址、端口号等信息*/
    assert(ret != -1);
    ret = listen(sock,5);                   /*监听有无连接请求（监听队列长度设置为5）*/
    assert(ret != -1);                      //assert() 会向标准输出设备（显示器）打印一条错误信息，并调用 abort() 函数终止程序的执行

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd=accept(sock,(struct sockaddr *)&client,&client_addrlength);//从监听队列中取出第一个已完成的连接（接收第一个客户的请求）
    char buffer_recv[MAX]={0};
    char buffer_send[MAX]={0};
    // 因为是半双工，一次仅有一边接收和发出，所以不用fork()创建新进程
    while(1) {
        if(connfd < 0) {                                // 若连接失败
            printf("errno is : %d\n",errno);    
        }
        else {
            memset(buffer_recv,0,MAX);
            memset(buffer_send,0,MAX);                  /*每次需要为缓冲区清空*/

            ret = recv(connfd, buffer_recv, MAX-1, 0);  //接收客户端发来的信息
/*
ssize_t recv(int sockfd,void *buf, size_t len, int flags);（接收数据）
返回值参数与send函数相似（成功接收到的字符数）
不过send是将buf中的数据向外发送，而recv是将接收到的数据写到buf缓冲区中
*/
            if(strcmp(buffer_recv,"quit\n") == 0) {     
                printf("Communications is over!\n");
                break;
            }   /*recv为quit表示客户端请求断开连接，退出循环*/
            printf("client:%s", buffer_recv);

            printf("server:");
            fgets(buffer_send,MAX,stdin);
            send(connfd,buffer_send,strlen(buffer_send),0);
/*
ssize_t send(int sockfd,const void * buf,size_t len,int flags);（发送数据）
返回值：成功返回发送的字符数，失败返回-1
参数：buf为写缓冲区（send_buf），len为发送缓冲区的大小，flags为一个标志，如MSG_OOB表示有紧急带外数据等
*/
            if(strcmp(buffer_send,"quit\n") == 0) {
                printf("Communications is over!\n");
                break;
            } /*send为quit表示服务器请求断开连接，退出循环*/
        }
    }
     /* 关闭连接停止通信 */
    close(connfd);
    close(sock);
    return 0;
}
