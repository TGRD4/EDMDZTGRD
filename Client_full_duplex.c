/* 而客户端指需要向服务端发起连接请求，不需要绑定监听（所以需要在服务端开后执行） */
/*
（1）服务器（Server）：服务器由于不知道客户何时回请求建立连接，所以必须绑定端口之后进行监听（Socket、Bind、Listen）
（2）客户端（Client）：客户端只需要向服务器发起请求连接（connect），而不需要绑定与监听的步骤
（3）请求连接由客户端发起（主动打开），服务器接受连接请求（被动打开），会经过TCP三次握手过程；
    而断开连接服务器和客户端都可以自行断开，会经过TCP四次挥手的过程
*/
/* tcp客户端（全双工）*/ 
# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<sys/socket.h>
# include<arpa/inet.h>
# include<netinet/in.h>
# include<signal.h>

# define MAX 1024

/*处理系统调用中产生的错误*/
void error_print(char * ptr) {
    perror(ptr);
    exit(EXIT_FAILURE);  
}
/*处理通信结束时回调函数接收到的信号*/
void quit_tranmission(int sig) { 
    printf("recv a quit signal = %d\n",sig);
    exit(EXIT_SUCCESS);
}
int main(void) {

    /* 创建套接字文件并设置地址的基本信息 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        error_print("socket");
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = 5050;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //将地址转换为二进制数
    /*inet_aton("127.0.0.1",&servaddr.sin_addr);*/
//inet_addr不支持255.255.255.255，inet_aton支持255.255.255.255 （计算的都是网络字节序的二进制IP）
//网络字节序就是大端字节序（高位字节在前，低位字节在后）


    /* 连接服务器（打开套接字文件） */
    int conn;
    if((conn = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
        error_print("connect");
    }
    /* 创建多进程实现全双工通信（两端同时发送和接收信息）*/
    pid_t pid;
    pid = fork();
    if(pid == -1) {
        error_print("fork");        // perror()
    }
// 子进程
    /* 收数据 */
    if(pid==0) {        
        char recv_buf[MAX] = {0};
        while(1) {
            bzero(recv_buf,sizeof(recv_buf));                   // 清空字符串
            int ret = read(sockfd, recv_buf, sizeof(recv_buf)); // 将sockfd所指文件中的数据读入recv_buf字符串中，返回成功读取字符数
            if(ret == -1) {
                error_print("read");                            // 读取字符串失败
            }
            else if(ret == 0) {
                printf("server is close!\n");                   // 未从字符串中读取到东西
                break;  //子进程收到服务器端退出的信息（服务器Ctrl+C结束通信进程，read函数返回值为0，退出循环）
            }
            fputs("server:",stdout);
            fputs(recv_buf,stdout);     /*将收到的信息输出到标准输出stdout上*/
        }
        /* 关闭连接 */
        close(sockfd);                  /*子进程退出，通信结束关闭套接字*/
        kill(getppid(),SIGUSR1);        /*子进程结束，也要向父进程发出一个信号告诉父进程终止接收，否则父进程一直会等待输入*/
        exit(EXIT_SUCCESS);             /*子进程正常退出结束，向父进程返回EXIT_SUCCESS*/
    }
// 父进程（用fgets获取用户输入的字符串，利用write将字符串写入创建好的套接字文件中，另一个用户从套接字文件中获取打包好的数据）
    /* 发出打包好的数据 */
    else {             
        /* 设置quit_tranmission函数来处理SIGUSR1默认退出信号 */
        signal(SIGUSR1,quit_tranmission);   /*回调函数处理通信中断*/    //SIGUSR1用户自定义信号，默认进程终止（子进程继承父的默认属性）
        char send_buf[MAX] = {0};
        /* 如果服务器Ctrl+C结束通信进程，fgets获取的就是NULL(无输出)，否则就进入循环正常发送数据 */
        while(fgets(send_buf,sizeof(send_buf), stdin) != NULL) {    // 读取输入流中的数据到send_buf数组中
            int set = write(sockfd, send_buf, strlen(send_buf));    // 将send_buf缓冲区的数据发送给对端服务器
            // write()会把参数 buf 所指的内存写入 count 个字节到参数 fd 所指的文件（这里就是套接字文件）内.
            if(set < 0)
                error_print("write");
            bzero(send_buf,strlen(send_buf));       //清空字符串数组便于下次的输入
        }
        close(sockfd);  /*通信结束，关闭套接字文件*/
    }
    return 0;
}
/*
整个客户端之间的通信就是通过套接字文件socket的打开写入和读取关闭来实现的.
（两端都使用connet连接到同一个属性的套接字文件(需提前设置好socket的属性)进行信息的双向传输）
*/

