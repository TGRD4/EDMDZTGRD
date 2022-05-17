#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
pthread_cond_t cond;
pthread_mutex_t mutex;
#define MAX 10
/*
//Multi-producer , Multi-consumer Queue
struct MPMCQueue {
    // Define Your Data Here 
} typedef MPMCQueue;

MPMCQueue *MPMCQueueInit(int capacity);
void MPMCQueuePush(MPMCQueue *queue, void *s);
void *MPMCQueuePop(MPMCQueue *queue);
void MPMCQueueDestory(MPMCQueue *);
*/
typedef struct MPMCQueue {
    int num;
    int *p;
}MPMCQueue;

MPMCQueue *MPMCQueueInit(int capacity);
void MPMCQueuePush(MPMCQueue *queue, void *s);
void *MPMCQueuePop(MPMCQueue *queue);
void MPMCQueueDestory(MPMCQueue *queue);

MPMCQueue *MPMCQueueInit(int capacity) {
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
    MPMCQueue *queue=(MPMCQueue *)malloc(sizeof(MPMCQueue));
    queue->num=0;
   (queue->p)=(int *)malloc(sizeof(int)*capacity);      //queue->p=0 错误！
    return queue;
}

void MPMCQueuePush(MPMCQueue *queue, void *s) {
    /* 一边补充一边告诉消费者可以进行消费（边买边补） */
    /* 与单生产者消费者模式不同的就在于补货和消费的关系（共享空间是否需要保持满的状态） */
    while(1) {
        if(queue->num<MAX) {
            pthread_mutex_lock(&mutex);
            queue->p[queue->num]=rand()%100;
            printf("producer : %d\n",queue->p[queue->num]);
            (queue->num)++;
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&cond);
        }
        else {
            printf("the container is full!\n");
        }
        sleep(rand()%4);
    }
}

void *MPMCQueuePop(MPMCQueue *queue) {
    while(1) {
        pthread_mutex_lock(&mutex);
        if(queue->num<=0) {
            printf("wait for the offer!\n");
            pthread_cond_wait(&cond,&mutex);
        }        
        (queue->num)--;
        printf("consumer : %d\n",queue->p[queue->num]);
        pthread_mutex_unlock(&mutex);
        sleep(rand()%7);
    }
}

void MPMCQueueDestory(MPMCQueue *queue) {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    free(queue);
}

int main() {
    MPMCQueue *queue;
    queue=MPMCQueueInit(MAX);
    pthread_t pid1,pid2;
    srand(time(NULL));
    int flag=0;
    flag=pthread_create(&pid1,NULL,(void*)MPMCQueuePush,(void*)queue);   
    if(flag) {
        perror("create producer pthread ");
        exit(EXIT_FAILURE);
    }
    flag=pthread_create(&pid2,NULL,(void*)MPMCQueuePop,(void*)queue);    
    if(flag) {
        perror("create consumer pthread ");
        exit(EXIT_FAILURE);
    }
    /* 阻塞一直等到目标线程结束 */
    pthread_join(pid1,NULL);                      
    pthread_join(pid2,NULL);  
    /* 销毁线程 */
    MPMCQueueDestory(queue); 
    return 0;
}
