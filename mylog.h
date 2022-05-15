#ifndef _MY_LOG_
#define _MY_LOG_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>     //让函数能够接收可变参数(va_args)
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>      //errno

/* 日志级别 */
#define MY_LOG_INFO 1
#define MY_LOG_DEBUG 2
#define MY_LOG_WARN 3
#define MY_LOG_ERROR 4
#define MY_LOG_FATAL 5
#define MY_LOG_PATH "./log/"
#define MAX_NUM 10
#define MAX_SIZE 1024 * 1024
#define SIZE 1024 * 4
#define MY_LOG_OUTPUT true

static pthread_mutex_t mylogMutex = PTHREAD_MUTEX_INITIALIZER;       //互斥锁初始化
//  省略符参数（任意形参）
#define my_log_printf(level, output, ...) log_printf(level, output, __FILE__, __LINE__, __VA_ARGS__)
static void log_printf(unsigned char level, bool output, const char *file, const long line, const char *fmt, ...)
{                       //  2 true str
                        //  2 true str number 
    if(level<MY_LOG_INFO) {     //1
        return;
    }
    pthread_mutex_lock(&mylogMutex);     /* 日志部分上锁 */ //保证线程安全
    //mylogMutex是一把互斥锁

    char temp_buf[SIZE]={0};
    va_list args;           //定义一个指向函数参数的指针
    int flag;
    va_start(args, fmt);    //宏初始化定义的va_list变量，使其指向第一个可变参数的地址
    flag=vsnprintf(temp_buf, SIZE, fmt, args);
/*
函数原型：int vsnprintf(char *str, size_t size, const char *format, va_list ap);
函数说明：将可变参数格式化输出到一个字符数组
参数：str输出到的数组，size指定大小，防止越界，format格式化参数，ap可变参数列表函数用法
返回值：返回成功写入的字符数
*/
    va_end(args);           //宏结束可变参数的获取
    int cnt=0;
    char buf[SIZE]={};
    char temp[SIZE]={};
    char FileName[SIZE]={};
    char FileNameTemp[SIZE]={};
    char FileNameLast[SIZE]={};
    char FileNameOld[SIZE]={};
    char tempfile[SIZE];
    FILE *fp = NULL;
    time_t time_now;
    struct tm *p;
    time(&time_now);           //返回自1970到如今经过的秒数
    p=localtime(&time_now);  //获取当前系统时间结构体
    getcwd(tempfile,SIZE);
    //将要保存的日志信息和时间戳信息整合
    memset(buf, 0, sizeof(buf));
    char order[SIZE]={};
    switch(level) {
        case 1:strcpy(order,"INFO");break;
        case 2:strcpy(order,"DEBUG");break;
        case 3:strcpy(order,"WARN");break;
        case 4:strcpy(order,"ERROR");break;
        case 5:strcmp(order,"FATAL");break;
    }
    sprintf(buf, "[%s] %s ▶[%d-%02d-%02d %02d:%02d:%02d -> %s:%ld] : ", order,tempfile,
    (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, file, line);    //24小时制
//星期p->tm_wday（0表示星期天，6表示星期一）
    strcat(buf, temp_buf);    //str   //str number
    //strcat(buf, "\0");
    if(output==true) {
        puts(buf);
    }
    DIR *dir;
    struct dirent *ptr;
    if((dir=opendir(MY_LOG_PATH))==NULL) //打开日志的文件目录，如果没有则建立
    {
        sprintf(temp,"mkdir -p %s",MY_LOG_PATH);    //将命令传入temp数组(-p)确保目录名称存在，不存在的就建一个，不加也可
        system(temp);                               //通过system()系统调用执行创建文件夹的命令
        dir=opendir(MY_LOG_PATH);                   //打开创建的目录流
    }
    /*确定文件名*/
    while((ptr=readdir(dir))!=NULL)            //循环读取当前目录下到所有文件
    {
        if(strcmp(ptr->d_name,".")==0||strcmp(ptr->d_name,"..")==0) {   //跳过隐藏文件
            continue;
        }
        strcpy(FileNameTemp, ptr->d_name);          //储存正在运行的文件名到FileNameTemp数组中（暂时储存文件名的数组）
        if(strlen(FileNameTemp)<23) {               //文件名长度小于日期总长度则继续
            continue;
        }
        if (strcmp(FileNameTemp, FileNameLast) > 0) {   //找到最新的文件（文件名大）
            strcpy(FileNameLast, FileNameTemp);         //将最新日期的文件名字保存在FileNameLast中
        }
        if (strcmp(FileNameTemp, FileNameOld) < 0) {    //找到最老的文件（文件名小）
            strcpy(FileNameOld, FileNameTemp);          //将最老日期的文件名字保存在FileNameOld中
        }
        cnt++;              //统计符合要求的文件个数
    }
    closedir(dir);              //注意目录流的关闭
    sprintf(FileName,"%s%s",MY_LOG_PATH,FileNameLast);   //将最新的文件名字匹配到路径中去，如果没有用默认文件名
    fp=fopen(FileName,"r+");                  //以读的方式打开最新文件
    if(fp==NULL) {                                //若没有日志文件产生过，第一次创建（确定文件名为第一次创建文件的时间）
        sprintf(FileName,"%s%d-%02d-%02d_%02d-%02d-%02d.log",MY_LOG_PATH, 
        (1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
        fp=fopen(FileName,"w+");
    }
    /*若创建过相同时间形式的文件则不再重复创建而是向文件中加入新的日志内容*/
    else {
        fseek(fp, 0, 2);                //SEEK_END值为2（从文件末尾开始写入）
        if (ftell(fp)>=MAX_SIZE)  {     //如果大小已经超出限制（若文件内容指针位置已经超出文件规定字符最大范围）
            fclose(fp);                     //直接关闭当前文件
            if(cnt>=MAX_NUM) {   //如果日志文件的个数达到路限制10个，则按日期进行循环覆盖
                sprintf(FileName, "%s%s", MY_LOG_PATH, FileNameOld);
                remove(FileName);        //删除（移除）最老的一个日志文件
                sprintf(FileName, "%s%d-%02d-%02d_%02d-%02d-%02d.log", MY_LOG_PATH, 
                (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
                //更新最新文件名为当前时间戳
            }
            else {                          //若文件个数未达到限制则直接更改日志文件名为当前时间戳        
                sprintf(FileName, "%s%d-%02d-%02d_%02d-%02d-%02d.log", MY_LOG_PATH, 
                (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
            }
            fp = fopen(FileName, "w+");      //以写的方式打开当前文件（超出限制则重新创建一个文件）
        }
        else {
            fclose(fp);                     //关闭以只读的方式打开的文件
            fp=fopen(FileName, "a+");  //以追加的方式打开文件（未达到限制）
        }
    }
    fwrite(buf, 1, strlen(buf), fp);        //字符大小为 1 (char)
/*
定义函数：size_t fwrite(const void * ptr, size_t size, size_t nmemb, FILE * stream)
函数说明：fwrite() 用来将数据写入文件流中. 参数 stream 为 已打开（提前fopen()） 的文件指针, 参数 ptr 指向欲写入的数据地址, 
        总共写入的字符数以参数 (size*nmemb) 来决定. fwrite()会返回实际写入的 nmemb 数目
参数：ptr -- 这是指向要被写入的元素数组的指针
    size -- 这是要被写入的每个元素的大小，以字节为单位
    nmemb -- 这是元素的个数，每个元素的大小为 size 字节
    stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流
返回值：返回实际写入的 nmemb 数目
*/
    fflush(fp);     //立即刷新缓存区到指定文件流中（fflush()用于清空文件缓冲区，如果文件是以写的方式打开 的，则把缓冲区内容写入文件）
    fsync(fileno(fp));    //fsync()负责将参数 fd 所指的文件数据, 由系统缓冲区写回磁盘, 以确保数据同步
    //fdatasync(fileno(fp));//fileno()用来取得参数 stream 指定的文件流所使用的文件描述词（返回文件描述符）
/*
fdatasync只刷新数据到磁盘
fsync同时刷新数据和inode信息到磁盘（fsync的功能是确保文件fd所有已修改的内容已经正确同步到硬盘上，该调用会阻塞等待直到设备报告IO完成）
*/  
    fprintf(fp,"\nerrno:%d\t",errno);
    fprintf(fp,"%s\t",strerror(errno));     //显示错误信息

    fseek(fp,0L,SEEK_END);
    int size=ftell(fp);
    fprintf(fp,"\t%d KB\n",size);   //显示文件总KB

    fclose(fp);                     //别忘了关闭打开的文件
    pthread_mutex_unlock(&mylogMutex);       /* 写日志部分解锁（解锁互斥），保证线程的正常分配进行 */
}
#endif
