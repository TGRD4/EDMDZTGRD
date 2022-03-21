#include<assert.h>
#include<math.h>
#include"strbuf.h"

void strbuf_init(struct strbuf *sb, size_t alloc) {
	sb->buf=(char*)calloc(sizeof(char),alloc); 
	sb->alloc=alloc;
	sb->len=0;
}

void strbuf_attach(struct strbuf *sb, void *str, size_t len, size_t alloc) {
	sb->alloc=alloc;
	sb->len=len;
	sb->buf=(char*)str;
}

void strbuf_addstr(struct strbuf *sb, const char *s) {
    int len = strlen(s);
    if(sb->alloc<=(sb->len+len+1)) {
        strbuf_grow(sb,len+1);
    }
    memcpy((sb->buf+sb->len),s,len);
    sb->len+=len;
    sb->buf[sb->len]='\0';
}

void strbuf_release(struct strbuf *sb) {
	free(sb->buf);
    /*sb->alloc=0;
	sb->len=0;*/
}

void strbuf_swap(struct strbuf *a, struct strbuf *b) {
    struct strbuf t=*a;
    a->len=b->len;
    a->alloc=b->alloc;
    a->buf=b->buf;
    b->len=t.len;
    b->alloc=t.alloc;
    b->buf=t.buf;
}

int strbuf_cmp(const struct strbuf* first, const struct strbuf* second) {
    if (first->len>second->len)
        return 1;
    else if (first->len<second->len)
        return -1;
    else
        return 0;
}

char *strbuf_detach(struct strbuf *sb, size_t *sz) {
    char* t=sb->buf;
    *sz=sb->alloc;
    return t;
}

void strbuf_grow(struct strbuf *sb, size_t extra) {
    if(sb->len+extra<=sb->alloc)     //空间足够
        return;
    sb->alloc=sb->len+extra+1;
    sb->buf=(char*)realloc(sb->buf,sizeof(char)*sb->alloc);    //不够则加
}


void strbuf_add(struct strbuf* sb, const void* data, size_t len) {
    if(data==NULL) {
		return;
	}
	int max=sb->alloc;
	int sum=sb->len+len;
	if (max<sum) {
		strbuf_grow(sb,len+sb->len);
    }
	strcat(sb->buf,(char*)data);
    sb->len=sum;
}

void strbuf_addch(struct strbuf *sb, int c) {
    if(sb->len+2>=sb->alloc) {
        sb->buf=(char*)realloc(sb->buf,sizeof(char)*(sb->alloc)*2);
		sb->alloc*=2;
    }
    memcpy(sb->buf+sb->len,&c,2);
    sb->len++;
}

void strbuf_addbuf(struct strbuf *sb, const struct strbuf *sb2) {
    /*
    if(sb->alloc<=(sb->len+sb2->len+1)) {
        strbuf_grow(sb,sb2->len+1);
    }
    */
    strbuf_addstr(sb,sb2->buf);
}

void strbuf_setlen(struct strbuf *sb, size_t len) {
	sb->len=len;
	sb->buf[sb->len]='\0';
}

size_t strbuf_avail(const struct strbuf *sb) {
	if(sb -> alloc)
        return (sb->alloc)-(sb->len)-1;
    else
        return 0;
}

void strbuf_insert(struct strbuf *sb, size_t pos, const void *data, size_t len) {
    //strbuf_grow(sb,len+ab->alloc);
    char *t=(char*)malloc(sizeof(char)*(len+sb->alloc));//(len)
    memcpy(t,sb->buf,pos);
    memcpy(t+pos,data,len); 
    memcpy(t+pos+len,sb->buf+pos,sb->len-pos);
    memcpy(sb->buf,t,sb->len+len);
    free(t);//释放
}


void strbuf_reset(struct strbuf* sb) {
    for(int i=0;i<sb->len;i++) {
        *(sb->buf)=(char)'\0';
    }
    //sb->alloc=0;
    sb->len=0;
}

void strbuf_rtrim(struct strbuf *sb) {
    while(sb->buf[sb->len-1]==' '||sb->buf[sb->len-1]=='\t') {
        sb->len--;
    }
    sb->buf[sb->len]='\0';
}

void strbuf_ltrim(struct strbuf *sb) {
    char *t=sb->buf;
    while(*t=='\t'||*t==' ') {
        t++;
        sb->len--;
    }
    memmove(sb->buf,t,sb->len);//保证不重叠
}

void strbuf_remove(struct strbuf *sb, size_t pos, size_t len) {
    char *t=(char*)malloc(sizeof(char)*(len+pos));
    int end=sb->len-len;
    memcpy(t,sb->buf,pos);
    memcpy(t+pos,sb->buf+pos+len,end-pos);
    memcpy(sb->buf,t,end);//memmove
    strbuf_setlen(sb,end);
    free(t);//防止内存泄漏
}

int strbuf_getline(struct strbuf *sb, FILE *fp){
    int cnt=0,t;
    while(1) {
        if(((t=fgetc(fp))==EOF)||t=='\n')
            break;
        if(strbuf_avail(sb)<1)
            sb->buf=(char*)realloc(sb->buf,sizeof(char)*(sb->alloc+1));
        strbuf_addch(sb,t); 
        cnt++;
    }
    sb->len=cnt;
    return 1;
}

ssize_t strbuf_read(struct strbuf *sb, int fd, size_t hint) {   
    FILE *fp;
    char e;
    if(((fp=fdopen(fd,"r"))==NULL)||(e=fgetc(fp))==EOF)    //fdopen()会将参数fildes 的文件描述词, 转换为对应的文件指针后返回
        return sb->len;
    else {
        sb->alloc+=(hint?hint:8192);
        sb->buf=(char*)realloc(sb->buf,sizeof(char)*(sb->alloc));
        sb->buf[sb->len++]=e;
        while((e=fgetc(fp))!=EOF) {  // 持续赋值
            sb->buf[sb->len]=e;
            sb->len++;
        }
        sb->buf[sb->len]='\0';  // !
        return sb->len;
    }
}


/*
将长度为 len 的字符串 str 根据切割字符 terminator 切成多个 strbuf,并从结果返回，max 可以用来限定最大切割数量。返回 struct strbuf 的指针数组，数组的最后元素为 NULL
*/
struct strbuf **strbuf_split_buf(const char *str, size_t len, int terminator, int max) {    

    /*测试中 string 开头\0 ，需单独判断 *str 为 0 的情况，否则会因为 result[1]->buf 为 "\0" 而导致与 "\0\0  123 345  "不相等的情况*/
    if(*str=='\0') {
        struct strbuf **sb;  
        sb=(struct strbuf **)calloc(3,sizeof(struct strbuf*)); //分配返回字符串指针空间，同时留下最后一个NULL的空间
        for(int i=0;i<2;i++) {   //为每一切割段分配空间
            sb[i] = (struct strbuf*)calloc(1,sizeof(struct strbuf));
            strbuf_init(sb[i],20);
        }

        /*分别设置sb[0]/sb[1]的长度测试*/
        strbuf_setlen(sb[0] ,1);// "\0"
        strbuf_setlen(sb[1] ,13);// "\0\0  123 345  "

        char *t = (char*)"\0\0  123 345  "; //强制转换 
        memcpy(sb[1]->buf,t,13); 

        sb[2] = NULL;   //最后元素赋值
        return sb;
    }

    /*开头不为\0*/
    struct strbuf **ssb=NULL;
    char s[len+1];
    int cnt=0;
    char p[2];
    p[0]=(char)terminator;//强制转换
    p[1]='\0';
    memcpy(s,str,len+1);
    for(int i=0;i<len;i++) {
        if(s[i]=='\0')
            s[i]=' ';
    }
    char* q=strtok(s,p);    //字符串分割函数（返回分割后的字符串指针）
    while(q&&cnt<=max) {   
        int Len=strlen(q);  //保留每段分割长度
        for(int i=0;i<Len+1;i++) {
            if(q[i]==' ')
                q[i]='\0';  //标记分割点
        }
        struct strbuf* sb=(struct strbuf*)malloc(sizeof(struct strbuf));//对于分割出的每一段都单独用sb操作
        strbuf_init(sb,Len+1);
        strbuf_add(sb,q,Len);
        ssb=(struct strbuf**)realloc(ssb,sizeof(struct strbuf*)*(cnt+1));
        ssb[cnt++]=sb;
        q=strtok(NULL,p);
        if(cnt>=max) break;
    }
    ssb=(struct strbuf**)realloc(ssb,sizeof(struct strbuf*)*(cnt+1));  //为最后的元素分配空间
    ssb[cnt]= NULL;     //数组最后的元素为NULL
    return ssb;
}

/*
target_str : 目标字符串，str : 前缀字符串，strlen : target_str 长度 ，前缀相同返回 true 失败返回 false
*/
bool strbuf_begin_judge(char *target_str, const char *str, int strnlen) {
    if(strnlen==0) 
        return true;
    int len=strlen(str);
    char e=target_str[len];
    target_str[len]='\0';
    if(!strcmp(target_str, str))
        return true;
    target_str[len]=e;
    return false;
}

/*
target_str : 目标字符串，begin : 开始下标，end 结束下标。len : target_buf的长度，参数不合法返回 NULL. 下标从0开始，[begin, end)区间。
*/
char *strbuf_get_mid_buf(char *target_buf, int begin, int end, int len) {   
    if(begin<0||end>len||end<begin)
        return NULL;
    char *s=(char*)malloc(sizeof(char)*(end-begin+1));
    char e=target_buf[end];
    target_buf[end]='\0';
    strcpy(s,target_buf+begin);
    target_buf[end]=e;
    return s;
} 
