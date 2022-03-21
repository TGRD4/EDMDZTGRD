#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

void print(char *dir,int len)//进一次目录调用一次print
{
	DIR *dp;
	struct dirent *nowdir;
	struct stat buf;
    /*
    使用 opendir 函数打开一个目录流
    使用 readdir 函数来遍历目录条目
    */
	if((dp=opendir(dir))==NULL)    //opendir()用来打开参数 name 指定的目录, 并返回DIR*形态的目录流, 
    {
		//perror("opendir");//输出错误信息
		return ;
	}
	
	chdir(dir);//将当前的工作目录改变成dir所指的目录.
	while((nowdir=readdir(dp)))  //读出当前目录下的所有文件
    {
		lstat(nowdir->d_name,&buf);
        //stat和lstat的区别：当文件是一个符号链接时，lstat返回的是该符号链接本身的信息；而stat返回的是该链接指向的文件的信息

		if(S_ISDIR(buf.st_mode))  //判断是否为目录
		{
        	if(strcmp(nowdir->d_name,".")==0||strcmp(nowdir->d_name,"..")==0)  
            /*.  代表当前目录，也就是你目前打开的那个文件夹，
              .. 代表当前目录的上一级目录*/
				continue;	//如果当前目录或者上级目录未遍历完则继续 
			printf("%*s%s/\n",len,"",nowdir->d_name);//len长度假读(目录)
			print(nowdir->d_name,len+4);  //输出文件时相对与目录加4个空格位置（包含在目录中的文件）
		} 
        else
			printf("%*s%s\n",len,"",nowdir->d_name);//目录外的单独文件
	}
    closedir(dp);	//关闭当前操作的目录
	chdir("..");    //!返回上级目录读出其余子目录/文件
}

int main()
{
	char *nowdir=".";
	print(nowdir,0);    //当前根目录前无空格
	return 0;
}
