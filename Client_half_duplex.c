# include<sys/socket.h>
# include<netinet/in.h>
# include<arpa/inet.h>
# include<signal.h>
# include<assert.h>
# include<stdio.h>
# include<unistd.h>
# include<string.h>
# include<stdlib.h>

#define MAX 1024

int main (int argc,char * argv[]) {
    /* 设置地址和端口 */
    const char * ip = argv[1];          
    int port = atoi(argv[2]);                       // atoi用来将字符串转换为整数

    struct sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));  // 初始化（清空）字符串
    server_address.sin_family = AF_INET;

    inet_pton(AF_INET,ip,&server_address.sin_addr); // 将IP地址从字符串格式转换成网络地址格式
    server_address.sin_port = htons(port);          // 将一个无符号短整型数值转换为网络字节序，即大端模式

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);   
    assert(sockfd >= 0);

    int connfd = connect(sockfd, (struct sockaddr *)&server_address,sizeof(server_address));    //发出主动连接和服务端connect

    char buffer_recv[MAX] = {0};
    char buffer_send[MAX] = {0};
    while(1) {
        if(connfd < 0) {
            printf("connection failed\n");
        }
        else{
            /* 每次接发信息前都初始化接发字符串 */
            memset(buffer_send,0,MAX);
            memset(buffer_recv,0,MAX);

            printf("client:");
            fgets(buffer_send,MAX,stdin);                       // 获取输入信息到输入流
            send(sockfd, buffer_send, strlen(buffer_send), 0);  // 将输入流中的内容发送至服务端（打开socket文件向里面写入）
            if(strcmp(buffer_send,"quit\n") == 0) {             // 若发出的信息是quit则退出
                printf("Communications is over!\n");
                break;
            } /*send为quit表示客户端请求断开连接，退出循环*/

            int ret = recv(sockfd,buffer_recv,MAX-1,0);         // 接收服务端发过来的信息（打开socket文件从里面读取）
            if(strcmp(buffer_recv,"quit\n") == 0) {             // 若接收到的信息也是quit则也退出
                printf("Communications is over!\n");
                break;
            } /*recv为quit表示服务器请求断开连接，退出循环*/
            printf("server:%s",buffer_recv);

        }
    }   
    /* 关闭连接和套接字文件 */
    close(connfd);
    close(sockfd);
    return 0;
}
