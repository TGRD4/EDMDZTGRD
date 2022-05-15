/* 多个生产者和消费者 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct Msg                  //共享数据
{
    int num;                //食物个数
    struct Msg *next;      
};
struct Msg *head = NULL;

pthread_mutex_t mutex;// = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_data; // = PTHREAD_COND_INITIALIZER;

void *producter1(void *arg)
{
    while(1)
    {   
        struct Msg *p = malloc(sizeof(struct Msg));
        p->num = rand()%1000 + 1;
        printf("producter 1——————————————————>%d\n", p->num);              
        pthread_mutex_lock(&mutex);
        p->next = head;
        head = p;
        
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&has_data);
        sleep(1);
    }
}

void *producter2(void *arg)
{
    while(1)
    {   
        struct Msg *p = malloc(sizeof(struct Msg));
        p->num = rand()%1000 + 1;
        printf("producter 2——————————————————>%d\n", p->num);              
        pthread_mutex_lock(&mutex);
        p->next = head;
        head = p;
        
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&has_data);
        sleep(1);
    }
}

void *producter3(void *arg)
{
    while(1)
    {   
        struct Msg *p = malloc(sizeof(struct Msg));
        p->num = rand()%1000 + 1;
        printf("producter 3——————————————————>%d\n", p->num);              
        pthread_mutex_lock(&mutex);
        p->next = head;
        head = p;
        
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&has_data);
        sleep(1);
    }
}

void *consumer1(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex);
        struct Msg *q;
        while(head == NULL)
        {
            pthread_cond_wait(&has_data, &mutex);            //阻塞消费者线程，等待生产者生产产品
        }
        q = head;
        printf("consumer 1--->%d\n",q->num);
        head = q->next;
        free(q);

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void *consumer2(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex);
        struct Msg *q;
        while(head == NULL)
        {
            pthread_cond_wait(&has_data, &mutex);            //阻塞消费者线程，等待生产者生产产品
        }
        q = head;
        printf("consumer 2--->%d\n",q->num);
        head = q->next;
        free(q);

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void *consumer3(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex);
        struct Msg *q;
        while(head == NULL)
        {
            pthread_cond_wait(&has_data, &mutex);            //阻塞消费者线程，等待生产者生产产品
        }
        q = head;
        printf("consumer 3--->%d\n",q->num);
        head = q->next;
        free(q);

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

int main()
{
    srand(time(NULL));  
    pthread_mutex_init(&mutex, NULL);       //初始化互斥锁
    pthread_cond_init(&has_data, NULL);     //初始化条件变量

    pthread_t pid, pidd;
    int ret;
    ret = pthread_create(&pid, NULL, (void *)producter1, NULL);
    if(ret != 0)
    {
        printf("pthread_creat error: producter!\n");
    }
    ret = pthread_create(&pid, NULL, (void *)producter2, NULL);
    if(ret != 0)
    {
        printf("pthread_creat error: producter!\n");
    }
    ret = pthread_create(&pid, NULL, (void *)producter3, NULL);
    if(ret != 0)
    {
        printf("pthread_creat error: producter!\n");
    }
    ret = pthread_create(&pidd, NULL, (void *)consumer1, NULL);
    if(ret != 0)
    {
        printf("pthread_creat error: consumer!\n");
    }
    ret = pthread_create(&pidd, NULL, (void *)consumer2, NULL);
    if(ret != 0)
    {
        printf("pthread_creat error: consumer!\n");
    }
    ret = pthread_create(&pidd, NULL, (void *)consumer3, NULL);
    if(ret != 0)
    {
        printf("pthread_creat error: consumer!\n");
    }
    pthread_join(pid, NULL);
    pthread_join(pidd, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&has_data);

    return 0;
}
