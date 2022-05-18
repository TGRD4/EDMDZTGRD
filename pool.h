#ifndef _THREAD_POOL_
#define _THREAD_POOL_

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX 12          //最大容量线程数
#define MIN 4           //最多同时运行线程数
#define NUM MIN-1       //需要销毁的线程数(同时运行数-1)
#define MISSION 100     //最多任务数
/* 任务队列结构 */
typedef struct {
    void *number;               //任务执行个数
    void (*pools)(void *);      //任务执行函数
}mission;

/* 线程池结构 */
typedef struct { 

    /* 线程池基本信息 */
    pthread_t manager_pid;          //管理者线程ID（管理线程池，为不同的任务分配一个执行者线程(将不同的任务交付给不同的人去做)）
    pthread_t *worker_pid;          //执行者线程ID（不止一个线程同时执行所以用指针）
    int min;                        //最小线程数
    int max;                        //最大线程数
    int run;                        //正在运行的线程
    int able;                       //休眠的线程
    int kill;                       //需要杀死的线程

    // 线程池互斥锁
    pthread_mutex_t mutex_pool;     //整个线程池的互斥锁
    pthread_mutex_t mutex_run;      //在跑线程的专门互斥锁
    // 线程池条件变量
    pthread_cond_t manager;          
    pthread_cond_t worker;
    // 线程关闭状态
    bool close;                     //0是false ，false是开启，true代表关闭状态(除0之外的数)
    
    /* 任务队列信息 */
    mission *M;
    int mission_num;                //当前任务个数
    int queue_capacity;             //队列容量
    int queue_head;                 //队头
    int queue_last;                 //队尾
}POOL;

// 创建线程池并初始化
POOL *Create_Pool();
// 向线程池中添加任务
void Pool_mission(POOL* pool,void(*pools)(void*),void* number);

// 获取线程池中工作的线程的个数
int Pool_run(POOL* pool);
// 获取线程池中可用的线程的个数
int Pool_able(POOL* pool);

// 执行者线程任务函数（执行任务）
void* Worker(void* number);
// 管理者线程任务函数(分配任务)
void* Manager(void* number);

// 单个线程退出
void Exit(POOL* pool);
// 销毁线程池
int Pool_destory(POOL* pool);
// 获取当前在跑线程的pid
void pools (void *number);

#endif 
