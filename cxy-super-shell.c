#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <fcntl.h> 
#include <signal.h>

#define MAX 10
#define SIZE 256

int cnt;
char order[SIZE];        //保存当前的命令行输入
char argv[MAX][SIZE];   //根据命令行输入等到的命令参数
int argc;               //命令参数个数
int flag1;               //判断是否有重定向符号
int flag2;
char *file;             //记录重定向目标文件
char history[MAX][SIZE];

void show();
void exec_ls();
void exec_rm();
void exec_cd();
void exec_wc();
void exec_env();
void exec_cat();
void exec_pwd();
void exec_echo();
void exec_pipe();
void exec_copy();
void exec_grep();
void exec_move();
void exec_help();
void exec_mkdir();
void exec_touch();
//void exec_daemon();
void copy_argv(char* shell_argv[]);
void copy_argvv(char *shell_argv[],int i);

/* 将命令参数拷贝到当前命令中 并且判断重定向 */
void copy_argv(char* shell_argv[]) {
    for(int i=0;i<argc;i++) {
        shell_argv[i]=argv[i];
        if(strcmp(shell_argv[i],">")==0) {      //标记命令中有输出重定向
            flag1=1;
        }
        else if(strcmp(shell_argv[i],">>")==0) {
            flag1=2;
        }
        /*
        else if(strcmp(shell_argv[i],"&")==0) {
            flag2=1;
            break;
        }
        */
    }
    if(flag1==1) {                                  
        for(int i=0;i<argc;i++) {
            if(strcmp(shell_argv[i],">")==0) {  
                shell_argv[i]=NULL;
                file=shell_argv[i+1];         //截断重定向后的内容指向重定向文件
            }
        }
    }
    else if(flag1==2) {
        for(int i=0;i<argc;i++) {
            if(strcmp(shell_argv[i],">>")==0) { 
                shell_argv[i]=NULL; 
                file=shell_argv[i+1]; 
            }
        }
    }
}

/* 管道文件 */
void copy_argvv(char *shell_argv[],int i) {
    int k=0;
    for(;i<argc;i++,k++) {
        shell_argv[k]=argv[i];
        if(strcmp(shell_argv[k],">")==0) {      //标记命令中有输出重定向
            flag1=1;
        }
        else if(strcmp(shell_argv[k],">>")==0) {
            flag1=2;
        }
    }
/*
ls | wc -l > 1.txt
*/
    if(flag1==1) {                                
        for(int i=0;i<k;i++) {
            if(strcmp(shell_argv[i],">")==0) { 
                printf("%d\n",i); 
                file=shell_argv[i+1];         //截断重定向后的内容指向重定向文件
                shell_argv[i]=NULL;           //保留重定向前的内容作为命令返还
            }
        }
    }
    else if(flag1==2) {
        for(int i=0;i<k;i++) {
            if(strcmp(shell_argv[i],">>")==0) { 
                file=shell_argv[i+1]; 
                shell_argv[i]=NULL;  
            }
        }
    }
}

/* ls */
void exec_ls() {
    int fd=0;
    int pid=fork();
    if(pid>0) {
        waitpid(pid,NULL,0);    //父进程等待（等待任何子进程识别码为 pid 的子进程）
        return;
    }
    char* shell_argv[SIZE]={};
    copy_argv(shell_argv);
    if(flag1==1) {	            //如果命令中存在重定向
        fd=open(file,O_RDWR | O_CREAT | O_TRUNC,0644);
        //open 返回值：若所有欲核查的权限都通过了检查则返回 0 值, 表示成功, 只要有一个权限被禁止则返回-1
        dup2(fd,1);             //指定新文件描述符为1，1为标准输出，0为标准输入
    }
    else if(flag1==2) {
        fd=open(file,O_RDWR | O_APPEND,0644);
        //open 返回值：若所有欲核查的权限都通过了检查则返回 0 值, 表示成功, 只要有一个权限被禁止则返回-1
        dup2(fd,1);             //指定新文件描述符为1，1为标准输出，0为标准输入
    }
/*
定义函数：int dup2(int odlfd, int newfd);
函数说明：dup2()用来复制参数oldfd 所指的文件描述词, 并将它拷贝至参数newfd 后一块返回.
若参数newfd为一已打开的文件描述词,则newfd 所指的文件会先被关闭.
dup2()所复制的文件描述词, 与原来的文件描述词共享各种文件状态.
返回值：当复制成功时, 则返回最小及尚未使用的文件描述词. 若有错误则返回-1, errno 会存放错误代码.
*/
    execv("/bin/ls",shell_argv);
    /* execv()用来执行参数 path 字符串所代表的文件路径，第二个参数利用数组指针来传递给执行文件 */
}

/* pwd */
void exec_pwd() {
    char buffer[SIZE]={};
    getcwd(buffer,SIZE);
    printf("%s\n", buffer);     //输出当前操作文件路径
}

/* cd */
void exec_cd() {
    char home_path[SIZE]={"/home"};
    int sign=0;
    int i=2;
    for(;i<strlen(order);i++) {
        if(order[i]=='-') {
            sign=1;
            break;
        }
        else if(order[i]=='~') {
            sign=2;
            break;
        }
    } 
    if(argc!=2) return;             //若未指定切换路径则直接返回
    getcwd(history[cnt++],SIZE);
    if(sign==0) {
        chdir(argv[1]);             //若指定了则切换到指定路径
    }
    else if(sign==1) {
        chdir(history[cnt-2]);
    }
    else if(sign==2) {
        chdir(home_path);
    }
    exec_pwd();                      //显示切换后的操作路径
}

/* environ 打印当前用户的环境变量 */
void exec_env() {
	execlp("env","",NULL);	    //输出第一个参数argv[0]
}

/*
execlp()会从 PATH 环境变量所指的目录中查找符合参数 file("env") 的文件名,找到后便执行该文件,
然后将第二个以后的参数当做该文件的argv[0],argv[1]……, 最后一个参数必须用空指针(NULL)作结束
返回值：
如果执行成功则函数不会返回, 执行失败则直接返回-1, 失败原因存于 errno 中
errno:
Linux 中系统调用的错误都存储于 errno 中，errno 由操作系统维护，存储就近发生的错误，即下一次的错误码会覆盖掉上一次的错误
*/

/* echo 在屏幕上显示参数并换行 */
void exec_echo() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    int fd=0;
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv);
    if(flag1==1) {
        fd=open(file,O_RDWR | O_CREAT | O_TRUNC,0644);
        dup2(fd,1);  
    }
    else if(flag1==2) {
        fd=open(file,O_RDWR | O_APPEND,0644);
        dup2(fd,1);          
    }
    execvp("echo",shell_argv);
} 

/* 执行 help */
void exec_help() {
    int pid=fork();
    if(pid>0) {
        waitpid(pid,NULL,0);
        return;
    }  
    system("help");     //此处调用系统的help显示在屏幕上
}

/* touch 创建文件 */
void exec_touch() {  
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv);    
    execvp("touch",shell_argv);  
}

/* mkdir 创建文件夹 */
void exec_mkdir() {  
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv);    
    execvp("mkdir",shell_argv);  
}

/* rm 删除文件夹和文件夹 */
void exec_rm() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv);    
    execvp("rm",shell_argv); 
}

/* cat 显示文档内容 */
void exec_cat() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv);    
    execvp("cat",shell_argv); 
}

/* cp 文件内容复制 */
void exec_copy() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv);    
    execvp("cp",shell_argv); 
}

/* move 移动 */
void exec_move() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv);    
    execvp("mv",shell_argv); 
}

/* grep 查询信息 */
void exec_grep() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    int fd=0;
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv); 
    if(flag1==1) {
        fd=open(file,O_RDWR | O_CREAT | O_TRUNC,0644);
        dup2(fd,1);  
    }
    else if(flag1==2) {
        fd=open(file,O_RDWR | O_APPEND,0644);
        dup2(fd,1);          
    } 
    execvp("grep",shell_argv); 
}

/* wc 显示信息 */
void exec_wc() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    int fd=0;
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv); 
    if(flag1==1) {
        fd=open(file,O_RDWR | O_CREAT | O_TRUNC,0644);
        dup2(fd,1);  
    }
    else if(flag1==2) {
        fd=open(file,O_RDWR | O_APPEND,0644);
        dup2(fd,1);          
    }  
    execvp("wc",shell_argv); 
}

/* pipe 管道 */
void exec_pipe() {
    pid_t pid=0;
    int fdd=0;
    int fd[2]={0};
    char* now_file[SIZE]={};
    char* next_file[SIZE]={};
    int i=0;
    pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }
    for(;strcmp(argv[i],"|")!=0;i++) {
        now_file[i]=argv[i];
    }
    now_file[i]=NULL;
    i++;
    int k=0;
    for(;i<argc;k++,i++) {
        next_file[k]=argv[i];
        if(strcmp(argv[i],">")==0) {
            flag1=1;
            next_file[k]=NULL;
            file=argv[i+1];
        }
        else if(strcmp(argv[i],">>")==0) {
            flag1=2;
            next_file[k]=NULL;
            file=argv[i+1];
        }
    }
    if(flag1==1) {	            //如果命令中存在重定向
        fdd=open(file,O_RDWR | O_CREAT | O_TRUNC,0644);
        //open 返回值：若所有欲核查的权限都通过了检查则返回 0 值, 表示成功, 只要有一个权限被禁止则返回-1
        dup2(fdd,1);             //指定新文件描述符为1，1为标准输出，0为标准输入
    }
    else if(flag1==2) {
        fdd=open(file,O_RDWR | O_APPEND,0644);
        //open 返回值：若所有欲核查的权限都通过了检查则返回 0 值, 表示成功, 只要有一个权限被禁止则返回-1
        dup2(fdd,1);             //指定新文件描述符为1，1为标准输出，0为标准输入
    }

    if(pipe(fd)==-1) {          //创建一个管道
        perror("pipe ");
    } 
    else {
        pid=fork();
        if(pid==0) { 
            dup2(fd[1],1);
            execvp(now_file[0],now_file);
        }
        else {     
            dup2(fd[0],0);      //从f[0]读出f[1]写入的内容到标准输入
            close(fd[1]); 
            execvp(next_file[0],next_file);
        }
    }
}
/*
ls -l -a | wc -l > 2.txt
ls | wc -l > 2.txt
*/

/* 显示 shell 提示内容 */
void show() {
	char path[SIZE]={};  	        //获取当前路径
	getcwd(path,SIZE);
    struct passwd *pw;      
    pw=getpwuid(getuid());          //返回当前用户数据的passwd结构体
	printf("\033[1;30m%s: \033[1;35m~%s \033[1;33m%c \033[0m",pw->pw_name,path,'$');
}
/*
getpwuid()用来逐一搜索参数 uid 指定的用户识别码,
找到时便将该用户的数据以结构返回结构请参考将该用户的数据以 passwd 结构返回
*/

/* & 后台运行 */
/*
void exec_daemon() {
    int pid=fork();    
    if(pid>0) {       
        waitpid(pid,NULL,0);       
        return;
    }   
    int fd=0;
    char* shell_argv[SIZE]={};    
    copy_argv(shell_argv); 
    if(flag1==1) {
        fd=open(file,O_RDWR | O_CREAT | O_TRUNC,0644);
        dup2(fd,1);  
    }
    else if(flag1==2) {
        fd=open(file,O_RDWR | O_APPEND,0644);
        dup2(fd,1);          
    }  
    execvp("&",shell_argv); 
}
*/

/* 主函数 */
int main() {   
    signal(SIGINT,SIG_IGN);         //防止 ctrl+c 杀死进程（信号屏蔽）
    while (1) {
        int flag=0;
        show();
        fgets(order,sizeof(order),stdin);
/*
sscanf()
【参数】参数 str 为要读取数据的字符串；format 为用户指定的格式；argument 为变量，用来保存读取到的数据
【返回值】成功则返回参数数目，失败则返回-1，错误原因存于 errno 中
*/
        argc=sscanf(order,"%s%s%s%s%s%s%s%s%s%s",
            argv[0],argv[1],argv[2],argv[3],argv[4], 
            argv[5],argv[6],argv[7],argv[8],argv[9]); 
        for(int i=0;i<argc;i++) {
            if(strcmp(argv[i],"|")==0) {
                flag=1;
                exec_pipe();
            }
        }
        if(flag==1) {
            continue;
        }
        else {
            if (strcmp(argv[0],"exit") == 0) {
                break;
            } 
            else if(strcmp(argv[0],"./cxy-super-shell")==0) {
                printf("shell is running now!\n");
            }
            /*
            if(strcmp(argv[0],"&")==0) {
                exec_daemon();
            }
            */
            else if (strcmp(argv[0],"ls") == 0) {
                exec_ls();
            } 
            else if (strcmp(argv[0],"pwd") == 0) {
                exec_pwd();
            } 
            else if (strcmp(argv[0],"cd") == 0) {
                exec_cd();
            } 
            else if(strcmp(argv[0],"env")==0) {
                exec_env();
            }
            else if(strcmp(argv[0],"echo")==0) {
                exec_echo();
            }
            else if(strcmp(argv[0],"help")==0) {
                exec_help();
            }
            else if(strcmp(argv[0],"mkdir")==0) {
                exec_mkdir();	
            }
            else if(strcmp(argv[0],"touch")==0) {
                exec_touch();
            }
            else if(strcmp(argv[0],"cp")==0) {
                exec_copy();
            }
            else if(strcmp(argv[0],"mv")==0) {
                exec_move();
            }
            else if(strcmp(argv[0],"rm")==0) {
                exec_rm();
            }
            else if(strcmp(argv[0],"cat")==0) {
                exec_cat(); 
            }
            else if(strcmp(argv[0],"wc")==0) {
                printf("jinlewc\n");
                exec_wc();
            }
            else if(strcmp(argv[0],"grep")==0) {
                exec_grep();
            }
            else {
                printf("No such an order!\n");
            }
        }
    }
    return 0;
}

