/* 先调用服务端绑定端口进行监听直到客户端发来连接请求后进行连接 */
/* tcp服务端（全双工）*/
# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<sys/socket.h>
# include<arpa/inet.h>
# include<netinet/in.h>
# include<signal.h>
# define MAX 1024
/* 输出错误信息 */
void error_print(char * ptr) {
    perror(ptr);
    exit(EXIT_FAILURE);
}

/* 获取通信进程退出信息 */
void quit_tranmission(int sig) {
    printf("recv a quit signal = %d\n",sig);        // SIGUSR1默认为10，进程终止
    exit(EXIT_SUCCESS);
}

int main(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);   /*IPV4流式协议即TCP协议*/
    if(sockfd < 0) {
        error_print("socket");
    }
    /* 设置基本网络信息 */
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;                  /*IPV4*/
    servaddr.sin_port = 5050;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");      /*使用本地环回地址做测试*/
/*
127.0.0.1，通常被称为本地回环地址(Loopback Address)，不属于任何一个有类别地址类。它代表设备的本地虚拟接口，所以默认被看作是永远不会宕掉的接口。
在Windows操作系统中也有相似的定义，所以通常在安装网卡前就可以ping通这个本地回环地址。一般都会用来检查本地网络协议、基本数据接口等是否正常的。
(实际上：127.0.0.1 —> 127.255.255.254（去掉0和255） 的范围都是本地回环地址)
*/

    /*inet_aton("127.0.0.1",&servaddr.sin_addr);//与inet_addr函数作用相同*/
    //inet_aton()用来将参数cp 所指的网络地址字符串转换成网络使用的二进制的数字, 然后存于参数inp 所指的in_addr 结构中.成功返回 0 值

    /*setsockopt确保服务器不用等待TIME_WAIT状态结束就可以重启服务器，继续使用原来的端口号(实现两个端口不停的全双工通信)*/
    int on = 1;         //欲设置的值
    if( setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0) {
                        //SOL_SOCKET 存取socket层，SO_REUSEADDR 本地地址可重复利用
                        //on指向包含新选项值的缓冲，sizeof(on)指现选项的长度
        error_print("setsockopt");
    }
    
    /* 1.绑定本地Socket地址（相当于当前网络地址连接上了设置好的套接字文件） */
    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        error_print("bind");
    }
    /* 2.监听连接 */
    if(listen(sockfd, SOMAXCONN) < 0) {     //listen函数将一个socket注册为被动socket，用来监听等待其他socket的主动connect
        error_print("listen");
    }
    struct sockaddr_in peeraddr;            /*存储连接成功的客户端Socket信息*/
    socklen_t peerlen = sizeof(peeraddr);
    int conn;
    /* 3.接收监听队列第一个完成连接的请求 */
    // 调用 accept() 代表有空闲的连接，返回一个新的套接字文件描述符
    // 原套接字文件描述符依旧监听连接，新的则负责收发数据
    if((conn = accept(sockfd,(struct sockaddr*)&peeraddr,&peerlen)) < 0) {
        error_print("accept");
    }
    // 使用accept一直阻塞到客户使用connect连接到达
    // 现客户端和服务端已经连接
    // 后实现TCP三次握手(服务请求，服务响应，文件结束通知(send(),recv(),close()))
    pid_t pid;
    pid = fork();                               /*创建一个新的子进程*/
    if(pid == -1) {
        error_print("fork");
    }
    /* 4.收发信息 */
    /* 子进程向对端回复信息（利用新套接字文件 发出 数据并输出）*/

    if(pid == 0) {                              /*子进程中用来向客户端发送数据*/
    // singnal 设置一个函数来处理信号，即带有 sig 参数的信号处理程序
        signal(SIGUSR1,quit_tranmission);       /*回调函数处理通信中断（若接收到的是对端发过来的终止信号则输出退出信息后也退出）*/
        char send_buf[MAX]={0};
        /*如果客户端Ctrl+C结束通信进程，fgets获取的就是NULL，否则就进入循环正常发送数据*/
        while(fgets(send_buf, sizeof(send_buf), stdin) != NULL) {
            write(conn,send_buf,strlen(send_buf));  //将send_buf中的数据发送给对端服务器
            bzero(send_buf,strlen(send_buf));       /*发送完成清空发送缓冲区*/
        }
        exit(EXIT_SUCCESS);                     /*成功退出子进程*/
    }
    /* 父进程获取对端发送的信息（利用新套接字文件 收取 数据并输出） */

    else {
        char recv_buf[MAX]={0};
        while(1) {
            bzero(recv_buf,strlen(recv_buf));                       
            int ret = read(conn, recv_buf, sizeof(recv_buf));       /*读取conn连接发送过来的数据*/
            if(ret < 0)
                error_print("read");
            else if(ret == 0) {
                printf("client is close!\n");
                break;      //父进程收到服务器端退出的信息（服务器Ctrl+C结束通信进程，read函数返回值为0，退出循环）
            }
            fputs("client:",stdout);                    //将读取到的数据进行输出（对面发过来的数据）
            fputs(recv_buf,stdout);
        }
        kill(pid,SIGUSR1);  /*父进程结束，也要向子进程发出一个信号告诉子进程终止接收，否则子进程会一直等待输入*/
        //发出SIGUSR1信号给pid所指的进程（相当于终止所有进程识别码为pid子进程）
/*
1、pid>0 将信号传给进程识别码为pid 的进程
2、pid=0 将信号传给和目前进程相同进程组的所有进程
3、pid=-1 将信号广播传送给系统内所有的进程
4、pid<0 将信号传给进程组识别码为pid 绝对值的所有进程参数 sig 代表的信号编号可参考附录D
*/
    }
    /* 5.关闭网络连接 */
    close(conn);
    close(sockfd);
    return 0;
}
