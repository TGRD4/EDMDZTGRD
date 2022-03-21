/*ls -aisrtl*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<time.h>
#include<dirent.h>
#include<stdbool.h>
#include<pwd.h>//passwd
#include<grp.h>//group
#define SIZE 1024


/*
char* filemode(mode_t m,char* s);
int filelink(mode_t m,char* name);
char* filetime(time_t t,char* str);
void print(char* name);
bool big(char str);
int cmp(char* str1,char* str2);

*/
//解析文件权限
/*
    S_ISLNK (st_mode)   判断是否为符号连接
    S_ISREG (st_mode)   是否为一般文件
    S_ISDIR (st_mode)   是否为目录
    S_ISCHR (st_mode)   是否为字符装置文件/字符设备
    S_ISBLK (s3e)       是否为先进先出/块设备
    S_ISSOCK (st_mode)  是否为socket（套接字）/SOCKET文件.
    S_ISFIFO是否是一个FIFO文件
*/


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
/*
	S_IRUSR 所有者拥有读权限
	S_IXGRP 群组拥有执行权限
	S_IWUSR 所有者拥有写权限
	S_IROTH 其他用户拥有读权限
	S_IXUSR 所有者拥有执行权限
	S_IWOTH 其他用户拥有写权限
	S_IRGRP	群组拥有读权限
	S_IXOTH	其他用户拥有执行权限
	S_IWGRP	群组拥有写权限
*/
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

//判断文件属性（链接数）
int filelink(mode_t m,char* name)
{
	if(S_ISREG(m)) return 1;//若是一般文件则链接数为1（硬链接）
	else if(S_ISDIR(m)) 
	{
		int cnt=0;
		DIR* dp=opendir(name);//打开给定目录
		if(dp==NULL)
		{
			perror("opendir");//输出错误信息
			return -1;
		}
/*
函数perror()用于抛出最近的一次系统错误信息，其原型如下：
    void perror(char *string);
参数：string为要输出的错误信息。（参数string所指的字符串会先打印出，后面再加上错误原因字符串）
*/
		for(struct dirent* dr=readdir(dp);dr;dr=readdir(dp))
		{
			if(dr->d_type==DT_DIR)//dirent结构体dr类型若是目录则硬链接数+1
			{
				cnt++;
			}
		}
		closedir(dp);//关闭目录
		return cnt;//返回硬链接数
	}
}

//将最后修改时间转换为字符串形式（用结构体进行保存）
char* filetime(time_t t,char* str)
{
	struct tm* nt=localtime(&t); //转为tm结构体可以自己选择输出格式
/* 
	gmtime()和localtime()将time()获得的日历时间time_t结构体转换成tm结构体。
	其中gmtime()函数是将日历时间转化为世界标准时间（即格林尼治时间），并返回一个tm结构体来保存这个时间，
	而localtime()函数是将日历时间转化为本地时间。
*/
	sprintf(str,"%2d月%02d日 %02d:%02d",nt->tm_mon+1,nt->tm_mday,nt->tm_hour,nt->tm_min);
	return str;
}


//打印所有数据 
void print(char* name)
{
	struct stat buf={};
	struct passwd *passwd;
	struct group *group;

	passwd=getpwuid(getuid());//获取用户ID
	group=getgrgid(passwd->pw_gid);//获取组ID
	//getgrgid()用来依参数 gid 指定的组识别码逐一搜索组文件, 找到时便将该组的数据以 group 结构返回
	
	char s[SIZE]={};
	char* file=getcwd(s,sizeof(s));
/*
定义函数： char * getcwd(char * buf, size_t size);
函数说明： getcwd()会将当前的工作目录绝对路径复制到参数 buf 所指的内存空间，参数 size 为 buf 的空间大小。
*/


/*
定义函数: int stat(const char *file_name, struct stat *buf);
函数说明: 通过文件名filename获取文件信息，并保存在buf所指的结构体stat中
返回值:   执行成功则返回0，失败返回-1，错误代码存于errno
*/
	if(stat(name,&buf))
	{
		perror("stat");
		return;
	}

	char str[SIZE]={};
	printf("%12d",buf.st_ino);//-i   //
	printf("%6ld ",buf.st_blocks/2);//块数（除以2）//-s
	printf("%s ",filemode(buf.st_mode,str));//文件权限
	printf("%2d ",filelink(buf.st_mode,name));//文件属性
	printf("%s ",passwd->pw_name);//用户ID
	printf("%s ",group->gr_name);//组ID
	printf("%8ld ",buf.st_size);//文件大小
	printf("%s ",filetime(buf.st_mtime,str));//文件最后修改时间（buf.st_mtime获得的是自1970.1.1到文件最后修改时间的秒数）
	if(S_ISDIR(buf.st_mode))//若是目录（蓝）
            printf("\033[1;34m%s\033[0m",name);//34 blue
    else if(S_ISFIFO(buf.st_mode))//目录/FIFO文件（黄）
        printf("\033[40;33m%s\033[0m",name);//33 yellow 
    else if(buf.st_mode & S_IXUSR || buf.st_mode & S_IXGRP || buf.st_mode & S_IXOTH)//可执行文件（绿）
		printf("\033[1;32m%s\033[0m",name);//32 green 
	else 
        printf("%s",name);//普通文件直接打印 
    printf("\n"); 
} 

//判断字符是否大写
bool big(char str)
{
	if(str>='A'&&str<='Z') return true;
	return false;
}

//比较函数,如果是大写则在比较时加上32当作小写字母比较
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

int main()
{ 
	char* name[SIZE]={};//保存所有文件名称
	char s[SIZE]={};
	char* file=getcwd(s,sizeof(s));
	DIR* dp=opendir(file);
	
	if(dp==NULL)
	{
		perror("opendir");//显示文件打开错误信息
		exit(EXIT_FAILURE);
	}
	int cnt=0;
	struct dirent* dr=readdir(dp);
	for(;dr;dr=readdir(dp))
	{
		if(dr->d_name[0]=='.') 
			continue;//-a
		name[cnt++]=dr->d_name;
	}
	
	/*
	//对文件进行字母排序
	for(int i=0;i<cnt-1;i++) 
	{
		for(int j=i;j<cnt;j++)
		{
			if(cmp(name[i],name[j])==1)//若str1大则调换到后面
			{
				char* temp=name[i];
				name[i]=name[j];
				name[j]=temp;
			}
		}
	}
	*/
	
	//对文件进行时间排序 -t


	long *fileTime[SIZE]={};
	//struct stat buf={};
	for(int i=0;i<cnt;i++)
	{
		struct stat buf={};
		stat((char*)name[i],&buf);
		fileTime[i]=(long*)buf.st_mtime;
		for(int j=i+1;j<cnt;j++)
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

	//逆序 -r
	for(int i=0,j=cnt;i<cnt/2,j>cnt/2;i++,j--)
	{
		char *ttemp=name[i];
		name[i]=name[j];
		name[j]=ttemp;
	}


	for(int i=0;i<cnt;i++)
	{
		print(name[i]);
	}
	closedir(dp);
	return 0;
}


/*
struct stat
{
    dev_t st_dev; //device 文件的设备编号
    ino_t st_ino; //inode 文件的i-node
    mode_t st_mode; //protection 文件的类型和存取的权限
    nlink_t st_nlink; //number of hard links 连到该文件的硬连接数目, 刚建立的文件值为1.
    uid_t st_uid; //user ID of owner 文件所有者的用户识别码
    gid_t st_gid; //group ID of owner 文件所有者的组识别码
    dev_t st_rdev; //device type 若此文件为装置设备文件, 则为其设备编号
    off_t st_size; //total size, in bytes 文件大小, 以字节计算(字节数)
    unsigned long st_blksize; //blocksize for filesystem I/O 文件系统的I/O 缓冲区大小.（块大小）
    unsigned long st_blocks; //number of blocks allocated 占用文件区块的个数, 每一区块大小为512 个字节.（块数）
    time_t st_atime; //time of lastaccess 文件最近一次被存取或被执行的时间, 一般只有在用mknod、utime、read、write 与tructate 时改变.
    time_t st_mtime; //time of last modification 文件最后一次被修改的时间, 一般只有在用mknod、utime 和write 时才会改变
    time_t st_mtime; //time of last change i-node 最近一次被更改的时间, 此参数会在文件所有者、组、权限被更改时更新
};
*/
