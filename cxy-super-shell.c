#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
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
void ls_input();
int input();
void printt_ls();
void order(char *s);
void cd();
void sure_cd();
void mkdirr();
void mkcdd();
void touchh();
void my_grep();
void put_in();
void my_wc(char ch);
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

int cmp(char* str1,char* str2);
char* filemode(mode_t m,char* s);
int filelink(mode_t m,char* name);
char* filetime(time_t t,char* str);
bool big(char str);



char* filemode(mode_t m,char* s)
{
	if (S_ISREG(m))	s[0]='-';//一般文件
	else if (S_ISDIR(m)) s[0]='d';//目录
	else if (S_ISCHR(m)) s[0]='c';
	else if (S_ISBLK(m)) s[0]='b';
	else if (S_ISFIFO(m)) s[0]='q';
	else if (S_ISLNK(m)) s[0]='l';
	else if (S_ISSOCK(m)) s[0]='s';
	else s[0]='?';
	s[1]='\0';

	strcat(s,S_IRUSR&m?"r":"-");
	strcat(s,S_IWUSR&m?"w":"-");
	strcat(s,S_IXUSR&m?"x":"-");
	strcat(s,S_IRGRP&m?"r":"-");
	strcat(s,S_IWGRP&m?"w":"-");
	strcat(s,S_IXGRP&m?"x":"-");
	strcat(s,S_IROTH&m?"r":"-");
	strcat(s,S_IWOTH&m?"w":"-");
	strcat(s,S_IXOTH&m?"x":"-");
	return s;
}

int filelink(mode_t m,char* name)
{
	if(S_ISREG(m)) return 1;
	else if(S_ISDIR(m)) 
	{
		int ll_cnt=0;
		DIR* dp=opendir(name);
		if(dp==NULL)
		{
			perror("opendir ");
			return -1;
		}
		for(struct dirent* dr=readdir(dp);dr;dr=readdir(dp)) {
			if(dr->d_type==DT_DIR)
			{
				ll_cnt++;
			}
		}
		closedir(dp);
		return ll_cnt;
	}
}


char* filetime(time_t t,char* str)
{
	struct tm* nt=localtime(&t); 
	sprintf(str,"%2d月%02d日 %02d:%02d",nt->tm_mon+1,nt->tm_mday,nt->tm_hour,nt->tm_min);
	return str;
}

bool big(char str)
{
	if(str>='A'&&str<='Z') return true;
	return false;
}

int cmp(char* str1,char* str2)
{
	for(int i=0;str1[i]!='\0'&&str2[i]!='\0';i++)
	{
		if(str1[i]=='.'||str2[i]=='.') break;
		if((big(str1[i])&&big(str2[i]))||(!big(str1[i])&&!big(str2[i])))
		{
			if(str1[i]>str2[i]) return 1;
			else if(str1[i]<str2[i]) return 0;
		} 
		if(big(str1[i])&&!big(str2[i]))
		{
			if((str1[i]+32)>str2[i]) return 1;
			else if((str1[i]+32)<str2[i]) return 0;
		}
		if(!big(str1[i])&&big(str2[i]))
		{
			if(str1[i]>(str2[i]+32)) return 1;
			else if(str1[i]<(str2[i]+32)) return 0;
		}
	} 
	if(strlen(str1)>strlen(str2)) return 1;
	return 0;
}

/* ls - */
void printt_ls()
{
    int the_order[MAX]={};
    int ooder_sum=0;
    int ls_a=0;
    int ls_l=0;
    int ls_t=0;
    int ls_i=0;
    int ls_s=0;
    int ls_r=0;
    if(s[4]==' ') {
        printf("please input a right and standard order !\n");
        return;
    }
    else {
        int okok=0;
        for(int i=4;i<strlen(s)&&s[i]!=' ';i++) {
            the_order[ooder_sum++]=s[i];
            if(s[i]=='a') {
                ls_a=1;
            }
            else if(s[i]=='l') {
                ls_l=1;
            }
            else if(s[i]=='r') {
                ls_r=1;
            }
            else if(s[i]=='i') {
                ls_i=1;
            }
            else if(s[i]=='s') {
                ls_s=1;
            }
            else if(s[i]=='t') {
                ls_t=1;
            }
            else {
                printf("no such an order !\n");
                return ;
            }
        }
        char* name[MAX]={};
        int sum=0;
        DIR* dp=opendir(getcwd(path,sizeof(path)));
        if(dp==NULL) {
            perror("opendir ");
            return;
        }
        struct dirent* dr=readdir(dp);
        for(;dr;dr=readdir(dp))
        {
            if(dr->d_name[0]=='.'&&!ls_a) 
                continue;
            name[sum++]=dr->d_name;
        }
         
        /* 字母排序 */
        for(int i=0;i<sum-1;i++) {
		    for(int j=i;j<sum;j++) {
                if(cmp(name[i],name[j])==1) {
                    char* temp=name[i];
                    name[i]=name[j];
                    name[j]=temp;
                }
            }
        }
        
        /* -t 时间排序 */
        if(ls_t) {
            long *fileTime[SIZE]={};
            for(int i=0;i<sum;i++)
            {
                struct stat buf={};
                stat((char*)name[i],&buf);
                fileTime[i]=(long*)buf.st_mtime;
                for(int j=i+1;j<sum;j++)
                {
                    stat((char*)name[j],&buf);
                    fileTime[j]=(long*)buf.st_mtime;
                    if(fileTime[i]<fileTime[j])
                    {
                        long *t=fileTime[i];
                        fileTime[i]=fileTime[j];
                        fileTime[j]=t;
                        char *temp=name[i];
                        name[i]=name[j];
                        name[j]=temp;
                    }
                }
            }
        }

        /* -r 倒序 */
        if(ls_r) {
            for(int i=0,j=sum;i<sum/2,j>sum/2;i++,j--) {
                char *ttemp=name[i];
                name[i]=name[j];
                name[j]=ttemp;
            }
        }
        struct passwd *passwd;
	    struct group *group;
        struct stat buf={};
        passwd=getpwuid(getuid());
	    group=getgrgid(passwd->pw_gid);
        for(int j=0;j<sum;j++)
        {
            if(j%6==0&&j&&!ls_l) {
                printf("\n");
            }
            if(stat(name[j],&buf)) {
                perror("stat ");
                return;
            }
            char str[SIZE]={};
            if(ls_i)    printf("%12d ",buf.st_ino);//-i      
            if(ls_s)    printf("%6ld ",buf.st_blocks/2);//-s
            if(ls_l)    printf("%s ",filemode(buf.st_mode,str));//文件权限
            if(ls_l)    printf("%2d ",filelink(buf.st_mode,name[j]));//文件属性
            if(ls_l)    printf("%s ",passwd->pw_name);//用户ID
            if(ls_l)    printf("%s ",group->gr_name);//组ID
            if(ls_l)    printf("%8ld ",buf.st_size);//文件大小
            if(ls_l)    printf("%s ",filetime(buf.st_mtime,str));//文件最后修改时间
            if(S_ISDIR(buf.st_mode))//若是目录（蓝）
                    printf("\033[1;34m%s\033[0m",name[j]);//34 blue
            else if(S_ISFIFO(buf.st_mode))//目录/FIFO文件（黄）
                printf("\033[40;33m%s\033[0m",name[j]);//33 yellow 
            else if(buf.st_mode & S_IXUSR || buf.st_mode & S_IXGRP || buf.st_mode & S_IXOTH)//可执行文件（绿）
                printf("\033[1;32m%s\033[0m",name[j]);//32 green 
            else 
                printf("%s",name[j]);//普通文件直接打印 
            if(ls_l)  {
                okok=1;
                printf("\n"); 
            }
            else printf("    ");
            
        }
        closedir(dp);
        if(!okok) {
            printf("\n"); 
        }
        return ;
    }
}

/* ls >> */
void ls_input()
{
    char* name[MAX]={};
    int sum=0;
	DIR* dp=opendir(getcwd(path,sizeof(path)));
	if(dp==NULL) {
		perror("opendir ");
		return;
	}
	struct dirent* dr=readdir(dp);
	for(;dr;dr=readdir(dp))
	{
		if(dr->d_name[0]=='.') 
			continue;
		name[sum++]=dr->d_name;
	}
    int i=5;
    while(s[i]==' ') {
        i++;
    }
    char input_file[MAX]={};
    int sum_input=0;
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        input_file[sum_input++]=s[i];
    }
    FILE *fp;
    if(fp=fopen(input_file,"w")){
        for(int j=0;j<sum;j++) {
            if(j==sum-1) {
                fprintf(fp,"%s",name[j]);
            }
            else {
                fprintf(fp,"%s\n",name[j]);
            }
        }
    }
    else {
        perror("open file ");
        return;
    }
    fclose(fp);
    closedir(dp);
}

/* ls */
void print_ls(int flag)
{
    char* name[MAX]={};
    int sum=0;
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
	char str[SIZE]={};
	struct stat buf={};
	char* file=getcwd(s,sizeof(str));
	DIR* dp=opendir(file);
	if(dp==NULL)
	{
		perror("opendir ");
		return;
	}
	struct dirent* dr=readdir(dp);
	for(;dr;dr=readdir(dp))
	{
		if(dr->d_name[0]=='.') 
			continue;
		name[sum++]=dr->d_name;
	}
    int oo=0;
    for(int i=0;i<sum;i++)
	{
        oo=1;
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
    if(oo) {
        putchar('\n');
    }
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

    /* 嵌套 */
    if(strcmp(s,"./cxy-super-shell")==0) {
        printf("cxy-super-shell is running now !\n");
    }

    /* ls */
    else if(strcmp(s,"ls")==0) {
        print_ls(flag);
    }

    /* ls & */
    else if(strcmp(s,"ls &")==0) {
        flag=1;
        print_ls(flag);
    }
    
    /* ls >> */
    else if(memcmp(s,"ls >>",5)==0) {
        ls_input();
    }

    else if (memcmp(s,"ls -",4)==0) {
        printt_ls();
    }

    /* wc 显示文件信息 */
    else if(memcmp(s,"wc ",3)==0) {
        if(s[3]=='-') {
            my_wc(s[4]);
        }           
        else {
        }
    }
    
    /* grep 查找指定信息 */
    else if(memcmp(s,"grep ",5)==0) {
        my_grep();
    }

    /* mkdir 创建目录 */
    else if(memcmp(s,"mkdir ",6)==0) {
        mkdirr();
    }
    else if(memcmp(s,"mkcd ",5)==0) {
        strcpy(history[cnt++],path);
        mkcdd();
        strcpy(history[cnt++],path);
    }

    /* touch 创建文件 */
    else if(memcmp(s,"touch ",6)==0) {
        touchh();
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

    /* cd 指定目录 */
    else if(memcmp(s,"cd ",3)==0) {
        strcpy(history[cnt++],path);
        sure_cd();
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
        int len=strlen(s);
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

/* grep 查找指定信息 */
void my_grep() 
{
    int i=5;
    char grep_order[MAX]={};
    int sum_order=0;
    char grep_filename[MAX]={};
    int sum_f_name=0;
    while(s[i]==' '||s[i]=='\''||s[i]=='\"') {
        i++;
    }
    for(;i<strlen(s)&&s[i]!=' '&&s[i]!='\''&&s[i]!='\"';i++) {
        grep_order[sum_order++]=s[i];
    }
    while(s[i]==' '||s[i]=='\''||s[i]=='\"') {
        i++;
    }
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        grep_filename[sum_f_name++]=s[i];
    }
    FILE *fp=fopen(grep_filename,"r");
    if(fp==NULL) {
        perror("open file ");
        return;
    }
    char file_str[MAX]={};
    int count=0;
	while(fgets(file_str,sizeof(file_str),fp))//逐行循环读取文件，直到文件结束 
	{
		if(strstr(file_str,grep_order)) {
            int i=0;
            char ch=0;
            int j=0;
            while(ch=file_str[i++]) {
                for(;j<sum_order;) {
                    if(grep_order[j]==ch) {
			            printf("\033[1;31m%c\033[0m",grep_order[j]); 
                        j++;
                        if(sum_order==j) {
                            j=0;
                        }
                        break;
                    }
                    else {
                        j=0;
                        putchar(ch);
                        break;
                    }
                }
            }
		}
        else {
            printf("%s",file_str);
        }
	}
    putchar('\n');
    fclose(fp);
}

/* wc 显示文件信息 */
void my_wc(char ch) 
{
    int i=5;
    int e;
    char wc_name[MAX]={};
    int sum_wc_name=0;
    while(s[i]==' ') {
        i++;
    }
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        wc_name[sum_wc_name++]=s[i];
    }
    FILE *fp=fopen(wc_name,"r");
    if(fp==NULL) {
        perror("open file ");
        return;
    }
    if(ch=='c') {
        int wc_c=0; 
        while(!feof(fp)) {
            if((e=fgetc(fp))!=EOF) {
                wc_c++;
            }
        }
        printf("%d %s\n",wc_c,wc_name);
    }
    else if(ch=='l') {
        int wc_l=0;
        while(!feof(fp)) {
            if((e=fgetc(fp))=='\n'&&(e=fgetc(fp))!=EOF) {
                wc_l++;
            }
        }
        printf("%d %s\n",wc_l+1,wc_name);
    }
    else  {
        printf("no such an order!\n");
    }
    fclose(fp);
}

/* touch 创建文件 */
void touchh()
{
    char new_file[MAX]={};
    int filename_sum=0;
    int i=6;
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        new_file[filename_sum++]=s[i];
    }
    FILE *fp;
    if(fp=(fopen(new_file,"w"))) {
        printf("creat success !\n");
    }
    else  {
        perror("creat file ");
        return;
    }
    fclose(fp);
}

/* 创建新目录 */
void mkdirr()
{
    int newname_sum=0;
    char newdir_name[MAX]={};
    int i=6;
    while(s[i]==' ') {
        i++;
    }
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        newdir_name[newname_sum++]=s[i];
    }
    mkdir(newdir_name,0777);
}

/* 创建并切换到新目录 */
void mkcdd() 
{
    int newname_sum=0;
    char newdir_name[MAX]={};
    int i=5;
    while(s[i]==' ') {
        i++;
    }
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        newdir_name[newname_sum++]=s[i];
    }
    printf("%s\n",newdir_name);
    mkdir(newdir_name,0777);
    char new_path[SIZE]={};
    strcpy(new_path,path);
    strcat(new_path,"/");
    strcat(new_path,newdir_name);
    chdir(new_path);
    strcpy(path,new_path);
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
    //echo <1.txt> 3.txt
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
    else {
        for(int i=0;i<len;i++) {
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
        perror("open file ");
        return;
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
        perror("open file ");
        return;
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
            return;
        }
        if(fp2==NULL) {
            perror("open in_file");
            return;
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

/* cd 切换到指定目录 */
void sure_cd() 
{
	DIR *dp;
	struct dirent *dr;
	struct stat buf;
    int i=3;
    char cd_filename[MAX]={};
    int sum_cd=0;
    
    while(s[i]==' ') {
        i++;
    }
    for(;i<strlen(s)&&s[i]!=' ';i++) {
        cd_filename[sum_cd++]=s[i];
    }

    if((dp=opendir(cd_filename))==NULL) {
		perror("open a dir ");
		return ;
	}
    if(dr=readdir(dp)) {
        stat(dr->d_name,&buf);
        if(S_ISDIR(buf.st_mode)) {
            strcat(path,"/");
            strcat(path,cd_filename);
            chdir(cd_filename);
        }
    }
    else {
        printf("%s is not a dir !\n",cd_filename);
        return;
    }
    closedir(dp);
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
        perror("open file ");
        return;
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
        perror("open file ");
        return;
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
        perror("open file ");
        return;
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
        if(strcmp(s,"exit")==0) {
            printf("\033[1;30m%s\033[0m","Bye Bye ~\n");
            exit(0);
        }
        order(s);
    } 
    return 0;
}

/* 
ctrl + z 挂起 T
ctrl + c 中断 
*/
