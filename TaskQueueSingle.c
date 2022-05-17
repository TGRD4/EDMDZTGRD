#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define MAX 10
pthread_mutex_t mutex;
pthread_cond_t consume;
pthread_cond_t supply;
/*
// Single-producer , single-consumer Queue
struct SPSCQueue {
    // Define Your Data Here 

} typedef SPSCQueue;

SPSCQueue *SPSCQueueInit(int capacity);
void SPSCQueuePush(SPSCQueue *queue, void *s);
void *SPSCQueuePop(SPSCQueue *queue);
void SPSCQueueDestory(SPSCQueue *);
*/
typedef struct SPSCQueue {
    int num;
    int *p;
}SPSCQueue;

SPSCQueue *SPSCQueueInit(int capacity);
void SPSCQueuePush(SPSCQueue *queue, void *s);
void *SPSCQueuePop(SPSCQueue *queue);
void SPSCQueueDestory(SPSCQueue *queue);

/* 线程初始化 */
SPSCQueue *SPSCQueueInit(int capacity) {                        //给定了初始容量
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&consume,NULL);
    pthread_cond_init(&supply,NULL);

    SPSCQueue *queue=(SPSCQueue *)malloc(sizeof(SPSCQueue));    //分配共享空间
    queue->num=0;                                               //共享空间结构体初始化
    (queue->p)=(int *)malloc(sizeof(int)*capacity);
    return queue;
}
/* 或者这样初始化： 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   
pthread_cond_t supply = PTHREAD_COND_INITIALIZER; 
pthread_cond_t consume = PTHREAD_COND_INITIALIIZER;
*/

/* 生产者线程 */
void SPSCQueuePush(SPSCQueue *queue, void *s) {
    /* 一消费就补满并且等待补满后才解锁给消费者进行消费 */
    while(1) {                                      //用循环一直判断供求关系
        pthread_mutex_lock(&mutex);                 //上锁，进入生产
        if(queue->num<MAX) {                        //在未达到生产次数的时候一直进行生产
            queue->p[queue->num]=rand()%10;         //一次生产随机值
            printf("producer : %d\n",queue->p[queue->num]);
            (queue->num)++;                         //生产次数+1
            pthread_cond_signal(&supply);           //发出消费信号（存在产品可以进行消费）
        }
        else {   
            printf("the container is full!\n");     //若已经满足则停止生产
            pthread_cond_wait(&consume,&mutex);     //生产者一直等待消费者消费完毕的信号
        }
        pthread_mutex_unlock(&mutex);               //释放锁给消费者（促进消费）
        sleep(rand()%4);                            //休眠随机时间（生产者乘机进行一个休息）
    }
}

/* 消费者线程 */
void *SPSCQueuePop(SPSCQueue *queue) {
    while(1) {
        pthread_mutex_lock(&mutex);             //开始消费
        if(queue->num<=0) {                     //若无产品可以消费则等待生产者发来可以消费的信号
            printf("wait for the offer!\n");
            pthread_cond_wait(&supply,&mutex);
        }
        (queue->num)--;                         //进行一次消费
        printf("consumer : %d\n",queue->p[queue->num]);
        pthread_cond_signal(&consume);          //像消费者发出生产未满的信号要求生产者继续生产
        pthread_mutex_unlock(&mutex);
        sleep(rand()%7);
    }
}

/* 销毁线程 */
void SPSCQueueDestory(SPSCQueue *queue) {
    /* 销毁创建的互斥量和环境变量 */
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&supply);
    pthread_cond_destroy(&consume);
    //free(queue->q);
    free(queue);            //释放创建的结构体
}

int main() {  
    SPSCQueue *queue;
    queue=SPSCQueueInit(MAX);
    pthread_t pid1,pid2;
    srand(time(NULL));       //用当前时间作为初始化种子
    int flag=0;
    flag=pthread_create(&pid1,NULL,(void*)SPSCQueuePush,queue);     //创建生产者线程（必须是void*类型）
    if(flag) {
        perror("create producer pthread ");
        exit(EXIT_FAILURE);
    }
    flag=pthread_create(&pid2,NULL,(void*)SPSCQueuePop,queue);      //创建消费者线程（默认线程属性，queue作为实参传递给函数）
    if(flag) {
        printf("create consumer pthread ");
        exit(EXIT_FAILURE);
    }
    /* 获取某个线程执行结束时返回的数据 */
    pthread_join(pid1,NULL);                      
    pthread_join(pid2,NULL);  
    SPSCQueueDestory(queue); 
    return 0;
}
