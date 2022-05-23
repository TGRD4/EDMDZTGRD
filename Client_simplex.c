/* tcp 客户端（单工）*/
#include<stdio.h>
#include<sys/socket.h>      // 提供socket函数及数据结构
#include<netinet/in.h>      // 定义数据结构sockaddr_in
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>       // inet_addr()函数
#define SERV_PORT 5050      // 服务器端口（房屋的门）（5050代表门牌号）(分配一个空间进行通信)
#define MAX 1000
//地址（一个地区），端口（一间屋子）
int main(int argc,char *argv[]) {
    int sockfd, n;
    struct sockaddr_in servaddr,cliaddr;
/*
struct sockaddr_in {
    short int sin_family;           // 地址族 
    unsigned short int sin_port;    // 端口号 
    struct in_addr sin_addr;        // Internet地址 
    unsigned char sin_zero[8];      // 与struct sockaddr一样的长度 
};
*/
    char sendline[MAX];
    char recvline[MAX];
    if(argc != 2) {                             // 记得要设置地址参数
        printf("usage:server address!\n");
        exit(0);
    }
    sockfd = socket(AF_INET,SOCK_STREAM,0);     // 生成套接字并返回其文件描述符
/*
作用：创建一个socket，返回其文件描述符
原型：int socket(int domain,int type,int protocol);
参数：
domain：通信域， AF_INET：Ipv4 网络协议 ，AF_INET6：IPv6 网络协议，AF_UNIX：内核通信
type：SOCK_STREAM ：TCP协议，流式socket；SOCK_DGRAM：UDP协议，数据包socket
protocol：指定 socket 所使用的传输协议编号,通常为 0
返回值：成功则返回socket描述符，失败返回-1
*/

    /* 配置网路地址（设定要传递给的主机地址） */
    bzero(&servaddr,sizeof(servaddr));              // 先进行结构体初始化
/*
bzero() 会将内存块（字符串）的前n个字节清零，其原型为：
    void bzero(void *s, int n);
【参数】s为内存（字符串）指针，n 为需要清零的字节数
bzero()会将参数 s 所指的内存区域前 n 个字节，全部设为零值
实际上，bzero(void *s, int n) 等价于 memset((void*)s, 0,size_tn)，用来将内存块的前 n 个字节清零，
但是 s 参数为指针，又很奇怪的位于 string.h 文件中，也可以用来清零字符串
*/
    servaddr.sin_family = AF_INET;                  // 设置地址族
/*
地址族 简单来说就是底层是使用的哪种通信协议来递交数据的，如 AF_INET 用的是 TCP/IPv4；AF_INET6使用的是 TCP/IPv6；
而 AF_LOCAL 或者 AF_UNIX 则指的是本地通信（即本次通信是在当前主机上的进程间的通信）
*/
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);  // internet地址（相当于ip地址）
    //（带上当前主机地址127.0.0.1作为argv[1]传递给此程序）
    //（sockaddr_in 结构体包含着 sin_addr 结构体，s_addr 又是 sin_addr 结构体中的 uint32_t 类型的地址）
/*
定义函数：unsigned long int inet_addr(const char *cp);
函数说明：inet_addr()用来将参数cp 所指的网络地址字符串转换成网络所使用的二进制数字. 
网络地址字符串是以数字和点组成的字符串, 例如:"163. 13. 132. 68".
返回值：成功则返回对应的网络二进制的数字, 失败返回-1.
*/
/*
ip地址就是IP协议提供的一种统一的地址格式，它为互联网上的每一个网络和每一台主机分配一个逻辑地址，以此来屏蔽物理地址的差异
*/
    servaddr.sin_port = htons(SERV_PORT);           // 获取端口号
/*
uint16_t htons(uint16_t hostshort);
htons的功能：将一个无符号短整型数值转换为网络字节序，即大端模式(big-endian)
参数u_short hostshort: 16位无符号整数
返回值:TCP / IP网络字节顺序
(sockaddr_in结构体中sin_port成员类型是uint16_t)
*/
/*
所谓的端口，就好像是门牌号一样，客户端可以通过ip地址找到对应的服务器端，但是服务器端是有很多端口的，
每个应用程序对应一个端口号，通过类似门牌号的端口号，客户端才能真正的访问到该服务器
*/
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));          // 建立连接
/*
作用：用来请求连接一个指定ip和端口号的处于监听状态的被动socket
原型：int connect (int sockfd,struct sockaddr * serv_addr,int addrlen);     // sockaddr和sockaddr_in之间可以进行强制类型转换
参数：
sockfd：申请连接的主动socket的文件描述符
serv_addr：为结构体指针变量，存储着服务端被动socket的 IP 与端口号信息。
addrlen：表示serv_addr指向的结构体变量的长度
返回值：成功则返回 0，失败返回-1
*/
    while(fgets(sendline,MAX,stdin) != NULL) {              // 获取客户输入的字符串sendline 
        sendto(sockfd,sendline,strlen(sendline),0,(struct sockaddr *)&servaddr,sizeof(servaddr));   // 发送信息给Server服务器
/*
定义函数：int sendto(int s, const void * msg, int len, unsigned int flags, const struct sockaddr * to, int tolen);
函数说明：sendto() 用来将数据由指定的 socket（套接字文件(相当于打包了一个数据包送出)）传给对方主机.
参数s 为已建好连线的(connect成功) socket, 如果利用 UDP 协议则不需经过连线操作.（UDP不停进行数据包的传送(ping)，只要接受到反馈就证明相连） 
参数msg 指向欲连线的数据内容, 参数 flags 一般设0, 详细描述请参考send().（做默认）
参数to 用来指定欲传送的网络地址，参数tolen 为 sockaddr 的结果长度.
返回值：成功则返回实际传送出去的字符数, 失败返回－1, 错误原因存于 errno 中.
*/
        n=recvfrom(sockfd,recvline,MAX,0,NULL,NULL);     // 接收从服务器发来的信息
/*
定义函数：int recvfrom(int s, void *buf, int len, unsigned int flags, struct sockaddr *from,int *fromlen);
函数说明：recv()用来接收远程主机经指定的socket 传来的数据, 并把数据存到由参数buf 指向的内存空间, 参数len 为可接收数据的最大长度. 
参数flags 一般设0, 其他数值定义请参考recv(). 参数from 用来指定欲传送的网络地址, 结构sockaddr 请参考bind().
参数fromlen 为sockaddr 的结构长度.
返回值：成功则返回接收到的字符数, 失败则返回-1, 错误原因存于errno 中.
*/
        recvline[n]='\0';                                 // 处理消息字符串
        //fputs(recvline,stdout);                           // 输出从服务器接受的消息
    }
    close(sockfd);                                          // 关闭套接字
    return 0;
}
// S+(可中断的(位于后台的"+")睡眠状态)
/*
linux网络通信（客户端）
1.socket生成套接字文件用来在一个端口的两个对象之间传输数据
2.初始化包含需传输数据的结构体
3.设置数据结构体中的地址族，internet地址，端口号
4.connect建立两个对象之间的连接
5.获取用户输入到输入流中的字符串并将其发送到服务器
7.另一个用户端接受到服务器中的消息并进行处理后进行输出
8.关闭套接字从而停止两个对象之间的通信
*/
/*
tcp-客户端

1.创建一个socket，用函数socket()

2.设置要连接的对方的IP地址和端口等属性

3.连接服务器，用函数connect()

4.收发数据，用函数send()和recv()，或者read()和write()

5.关闭网络连接
*/
