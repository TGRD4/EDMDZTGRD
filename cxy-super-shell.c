#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>//passwd
#include <grp.h>//group
#include <signal.h>
#define SIZE 1024
#define MAX 100

int cnt;
char history[MAX][SIZE];
char path[SIZE];
char s[MAX];
void print_ls(int flag);
int input();
void order(char *s);
void cd();
void put_in();
void my_copy_cd();
void my_ocopy_cd();
void my_copy_cat(char *s);
void my_ocopy_cat(char *s);
void my_oocopy_cat(char *s);
void cat_inin(int _flag);
int check_file(char *file_name);
void write_in_file(char* file_name,char* my_input);
void add_in_file(char* file_name,char* my_input);
char *check_name(char *file_name);
char *check_order(char *my_input);
void clear_file(char* file_name);

/* ls */
void print_ls(int flag)
{
    /* ls & -- 单后台运行ls */
    if(flag) {
        if(daemon(1,1)<0)// 0：根目录 ; 1：当前目录 
        {
            perror("error daemon ");
            exit(1);
        }
	    sleep(4);
        putchar('\n');
        printf("[1] %d\n",getpid());
    }
    int sum=0;
    char* name[SIZE]={};
	char str[SIZE]={};
	struct stat buf={};
	char* file=getcwd(s,sizeof(str));
	DIR* dp=opendir(file);
	if(dp==NULL)
	{
		perror("opendir");
		return;
	}
	struct dirent* dr=readdir(dp);
	for(;dr;dr=readdir(dp))
	{
		if(dr->d_name[0]=='.') 
			continue;
		name[sum++]=dr->d_name;
	}
    for(int i=0;i<sum;i++)
	{
        if(stat(name[i],&buf))
	    {
		    perror("stat");
		    continue;
    	}
        if(S_ISDIR(buf.st_mode))//若是目录（蓝）
                printf("\033[1;34m%s    \033[0m",name[i]);//34 blue
        else if(S_ISFIFO(buf.st_mode))//目录/FIFO文件（黄）
            printf("\033[40;33m%s    \033[0m",name[i]);//33 yellow 
        else if(buf.st_mode & S_IXUSR || buf.st_mode & S_IXGRP || buf.st_mode & S_IXOTH)//可执行文件（绿）
            printf("\033[1;32m%s    \033[0m",name[i]);//32 green 
        else 
            printf("%s    ",name[i]);//普通文件直接打印 
        if((i+1)%10==0) {
            putchar('\n');
        }
	}
	closedir(dp);
    if(flag) {
        printf("\n[1]  + %d done       ls --color=tty",getpid());
        exit(0);//单后台运行后退出(否则占用进程空间导致系统变卡)
    }
    putchar('\n');
    return ;
}

/* 处理命令行 */
int input()
{
    fgets(s,MAX,stdin);
    s[strlen(s)-1]='\0';
    return strlen(s);
}

/* 识别命令 */
void order(char *s)
{
    int flag=0;
    char home_path[]="/home";
    /* ls */
    if(strcmp(s,"ls")==0) {
        print_ls(flag);
    }

    /* ls & */
    else if(strcmp(s,"ls &")==0) {
        flag=1;
        print_ls(flag);
    }

    /* cd - 历史目录 */
    else if(strcmp(s,"cd -")==0) {
        if(strcmp(history[0],"NULL")==0) {
            printf("no history!\n");
            return ;
        }
        else {
            cd(s);
            strcpy(history[cnt++],path);
        }
    }
    /* cd / 绝对路径 */
    else if(memcmp(s,"cd /",4)==0) {
        strcpy(history[cnt++],path);
        my_copy_cd();
        chdir(path);
        strcpy(history[cnt++],path);
    }
    /* cd ./ 相对路径 */
    else if(memcmp(s,"cd ./",5)==0) {
        strcpy(history[cnt++],path);
        my_ocopy_cd();
        chdir(path);
        strcpy(history[cnt++],path);
    }
    /* cd .. 返回上级目录 */
    else if(strcmp(s,"cd ..")==0) {
        strcpy(history[cnt++],path);
        chdir("..");
        char tempdir[SIZE];
        getcwd(tempdir,SIZE);
        strcpy(path,tempdir);
        strcpy(history[cnt++],path);
    }
    /* cd ~ 返回根目录(home) */
    else if(strcmp(s,"cd ~")==0) {
        strcpy(history[cnt++],path);
        strcpy(path,home_path);
        chdir(home_path);
        strcpy(history[cnt++],path);
    }

    /* cat */
    else if(memcmp(s,"cat /",5)==0) {
        my_copy_cat(s);
    }
    else if(memcmp(s,"cat ./",6)==0) {
        my_ocopy_cat(s);
    }
    else if(memcmp(s,"cat <<",6)==0) {
        put_in();
    }
    else if(memcmp(s,"cat <",5)==0) {
        int _flag=0;
        for(int i=5;i<strlen(s);i++) {
            if(s[i]=='>') {
                _flag=1;
            }
        }
        cat_inin(_flag);
    }
    else if(memcmp(s,"cat ",4)==0) {
        my_oocopy_cat(s);
    }

    /* echo */
    else if(memcmp(s,"echo ",5)==0) {
        char my_input[MAX]={};
        char file_name[MAX]={};
        int input_sum=0;
        int name_sum=0;
        int i=5;
        int len = strlen(s);
        int flag=0;
        while(s[i]==' ') {
            i++;
        }
        for(;i<len&&s[i]!='>';i++) {
            my_input[input_sum++]=s[i];
        }
        if(s[i]=='>') {
            i++;
            flag=1;
            if(s[i]=='>') {
                flag=2;
                i++;
            }
            while(s[i]==' ') {
                i++;
            }
            for(;i<len;i++) {
                file_name[name_sum++]=s[i];
            }
        }
        check_order(my_input);    

        if(flag==1) {
            clear_file(check_name(file_name));
            write_in_file(file_name,check_order(my_input));
        }
        else if(flag==2) {
            add_in_file(file_name,check_order(my_input));
        }
        else {
            printf("%s\n",check_order(my_input));   
        }
    }

    /* 不存在的命令 */
    else {
        printf("no such an order,try again!\n");
        return;
    }
    return;
}


/* 确定命令 */
char* check_order(char *my_input)
{
    int len=strlen(my_input);
    int cnt1=0,cnt2=0;
    while(my_input[len-1]==' ') {
        len--;
    }
    for(int i=0;i<len;i++) {
        if(my_input[i]=='\"') {
            cnt1++;
        }
        else if(my_input[i]=='\'') {
            cnt2++;
        }
    }
    int new_sum=0;
    char *new_input=(char*)calloc(MAX,sizeof(char));
    if(cnt1%2==0&&cnt1!=0) {
        for(int i=0;i<len;i++) {
            if(my_input[i]=='\"'||my_input[i]=='\\') {
                continue;
            }
            new_input[new_sum++]=my_input[i];
        }
    }
    else if(cnt2%2==0&&cnt2!=0) {
        for(int i=0;i<len;i++) {
            if(my_input[i]=='\''||my_input[i]=='\\') {
                continue;
            }
            new_input[new_sum++]=my_input[i];
        }
    }
    return new_input;
}

/* 确定文件名称 */
char* check_name(char *file_name)
{
    int i=strlen(file_name)-1;
    for(;file_name[i]==' ';i--);
    file_name[i+1]='\0';
    return file_name;
}

/* 文件内容删除 */
void clear_file(char* file_name)
{
    FILE * fp;
    fp = fopen (file_name, "w");
    if(fp==NULL) {
        perror("open file");
    }
    fclose(fp);
    return;
}

/* > 输出重定向 */ 
void write_in_file(char* file_name,char* my_input) 
{
    int flag=0;
    flag = open(file_name,O_WRONLY);
    if(flag) {
        write(flag, my_input, strlen(my_input));
        close(flag);
    }
    return ;
}

/* >> 输出重定向 */
void add_in_file(char* file_name,char* my_input)
{
    FILE * fp;
    fp = fopen(file_name,"a");
    if(fp==NULL) {
        perror("open file");
    }
    else {
        fprintf(fp, my_input,file_name);
    }
    fclose(fp);
    return ;
}

/* </> 输入/输出重定向 */
void cat_inin(int _flag)
{
    /* <>输入输出重定向 */
    if(_flag) {
        int i=5;
        char out_file[MAX]={};
        char in_file[MAX]={};
        int out_sum=0;
        int in_sum=0;
        for(;s[i]!='>';i++) {
            if(s[i]==' ') {
                continue;
            }
            out_file[out_sum++]=s[i];
        }
        i++;
        while(s[i]==' ') {
            i++;
        }
        for(;i<strlen(s)&&s[i]!=' ';i++) {
            in_file[in_sum++]=s[i];
        }
        char ch;
        FILE* fp1 = fopen(out_file, "r");
        FILE* fp2 = fopen(in_file, "w");
        if(fp1==NULL) {
            perror("open out_file");
        }
        if(fp2==NULL) {
            perror("open in_file");
        }
        while ((ch=fgetc(fp1))!=EOF)
        {
            fputc(ch,fp2);
        }
        fclose(fp1);
        fclose(fp2);
    }
    else {
        int sum_ss=0;
        char ss[MAX]={};
        for(int i=0;i<strlen(s);i++) {
            if(s[i]=='<') {
                continue;
            }
            ss[sum_ss++]=s[i];
        }
        order(ss);
    }
    return;
}

/* << 输入重定向 */
void put_in()//遇到一次空格后面的就不接受了
{
    int i=6;
    while(s[i]==' ') {
        i++;
    }
    int sum_save=0;
    char save[MAX][SIZE]={};
    char sign[MAX]={};
    char in_put[MAX]={};
    int sign_sum=0; 
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        sign[sign_sum++]=s[i];
    }
    do {
        printf("heredoc> ");
        scanf("%s",&in_put);
        strcpy(save[sum_save++],in_put);
    }while(strcmp(sign,in_put)!=0);
    for(int j=0;j<sum_save-1;j++) {
        printf("%s\n",save[j]);
    }
    getchar();
    return;
}

/* 返回历史目录 */
void cd()
{
    chdir(history[cnt-2]);
    strcpy(path,history[cnt-2]);
    return ;
}

/* cd 绝对路径 */
void my_copy_cd()
{
    int n=0;
    char opath[SIZE]={};
    for(int i=3;i<strlen(s);i++) {
        opath[n++]=s[i];
    }
    strcpy(path,opath);
    return;
}

/* cd 相对路径 */
void my_ocopy_cd()
{
    int n=0;
    char opath[SIZE]={};
    for(int i=4;i<strlen(s);i++) {
        opath[n++]=s[i];
    }
    strcat(path,opath);
    return;
}

/* cat 绝对路径 */
void my_copy_cat(char *s)
{
    int n=0;
    char opath[SIZE]={};
    for(int i=4;i<strlen(s);i++) {
        opath[n++]=s[i];
    }
    FILE *fp=fopen(opath,"r");
    if(fp==NULL) {
        perror("open file");
    }
    while(!feof(fp)) {
        int ch;
        if((ch=fgetc(fp))!=EOF) {
            putchar(ch);
        }
    }
    putchar('\n');
    fclose(fp);
    return;
}

/* cat 相对路径 */
void my_ocopy_cat(char *s)
{
    int n=0;
    char opath[SIZE]={};
    for(int i=6;i<strlen(s);i++) {
        opath[n++]=s[i];
    }
    FILE *fp=fopen(opath,"r");
    if(fp==NULL) {
        perror("open file");
    }
    while(!feof(fp)) {
        int ch;
        if((ch=fgetc(fp))!=EOF) {
            putchar(ch);
        }
    }
    putchar('\n');
    fclose(fp);
    return;
} 

/* cat 指定文件 */
void my_oocopy_cat(char *s)
{
    char opath[SIZE]={};
    int n=0;
    int flag=0;
    for(int i=4;i<strlen(s);i++) {
        if(s[i]==' ') {
            continue;
        }
        if(s[i]=='&') {
            flag=1;
            break;
        }
        opath[n++]=s[i];
    }
    /* cat & "filenam" -- 单后台运行显示指定文件 */
    if(flag) {
        if(daemon(1,1)<0)// 0：根目录 ; 1：当前目录 
        {
            perror("error daemon ");
            exit(1);
        }
	    sleep(4);
        putchar('\n');
    }
    FILE *fp=fopen(opath,"r");
    if(fp==NULL) {
        perror("open file");
    }
    while(!feof(fp)) {
        int ch;
        if((ch=fgetc(fp))!=EOF) {
            putchar(ch);
        }
    }
    if(flag) {
        printf("\n[1]  + %d done       cat %s",getpid(),opath);
        exit(0);//后台运行后退出
    }
    putchar('\n');
    fclose(fp);   
    return;
}

/* 判断路径所指文件/目录属性 */
int check_file(char *file_name)
{
    struct stat st;
    stat(file_name,&st);
    if(S_ISDIR(st.st_mode))
        return 1;
    return 0;
}

/* 主函数 */
int main()
{
    printf("\033[1;33m%s\033[0m","Please input the Standard format order ! thanks!\n");
    //signal(SIGINT,SIG_IGN); //防止 ctrl+c 杀死进程（信号屏蔽）
    strcpy(history[0],"NULL"); //历史目录初始化
    getcwd(path,SIZE); //获取当前路径
    while(1) {
        printf("\033[1;35m[%s] $ \033[0m",path);
        if(!input(s)) {
            continue;
        }
        order(s);
    } 
    return 0;
}
/* 
ctrl + z 挂起 T
ctrl + c 中断 
*/
