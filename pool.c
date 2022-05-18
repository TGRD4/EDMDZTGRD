#include "pool.h"
/* 创建并初始化线程池（返回创建成功的线程池） */
POOL *Create_Pool() {
    POOL *pool=NULL;
    while(1) {
        /* 线程池 */
        // 线程池空间分配
        POOL *pool=(POOL*)malloc(sizeof(POOL));                        
        if(pool==NULL) {
            perror("create pool "); 
            break;
        }
        // 线程池线程状态确定
        pool->min=MIN;
        pool->max=MAX;
        // 不用为管理者线程单独分配空间（pthread_t(unsigned long int)而并非数组）
        pool->worker_pid=(pthread_t *)malloc(sizeof(pthread_t)*MAX);    // 为执行者线程分配空间（最多存在MAX个执行线程）
        if(pool->worker_pid==NULL) {
            perror("allocate worker_pid space ");
            break;
        }

        // 线程队列初始化
        pool->kill=0;                           // 暂时不存在需要杀死的线程
        pool->able=MIN;                         // 剩余可用线程为同时能够运行的线程数
        pool->run=0;                            // 当前无在跑的线程
        pool->close=false;                      // 开启线程

        // 初始化互斥锁和条件变量
        pthread_mutex_init(&pool->mutex_pool,NULL);
        pthread_mutex_init(&pool->mutex_run,NULL);
        pthread_cond_init(&pool->worker,NULL);
        pthread_cond_init(&pool->manager,NULL);        
        // 创建线程
        pthread_create(&pool->manager_pid,NULL,Manager,pool);       // 创建管理者线程(单)
        for(int i=0;i<MIN;i++) {                                    // 同时工作的最多只能有MIN个执行者
            pthread_create(&pool->worker_pid[i],NULL,Worker,pool);  // 执行者(多)
        }
        
        /* 任务队列 */
        // 任务队列空间分配
        pool->M=(mission *)malloc(sizeof(mission)*MISSION);         // 别忘记乘以最多任务数
        if(pool->M==NULL) {
            perror("allocate mission space ");
            break;
        }
        // 任务队列初始化
        pool->queue_capacity=MISSION;           // 剩余空闲任务队列容量为最大任务数
        pool->mission_num=0;                    // 当前无任务
        pool->queue_head=0;                     // 无第一个任务
        pool->queue_last=0;                     // 也无最后一个任务
        return pool;                            // 返回创建成功的线程池
    }

    // 若创建失败则 释放掉分配的线程池结构体空间 后返回一个空线程池
    if(pool) {
        free(pool);
    }
    return NULL;
}

/* 向线程中添加任务 */
void Pool_mission(POOL* pool,void(*pools)(void*),void* number) {  
    if(pool->mission_num==pool->queue_capacity&&!pool->close) {         // 任务队列满了 但是线程池没有关闭（就是开启 就是0（false））
        pthread_cond_wait(&pool->manager,&pool->mutex_pool);            // 管理者发出信号阻塞线程池互斥量 从而暂停分配任务
    }
    if(pool->close) {                                                   // 线程池已经关闭（任务队列满了条件省略）
        pthread_mutex_unlock(&pool->mutex_pool);                        // 解锁线程池后退出（开启时锁定了线程池）
        return;                                                         // 不添加任务而直接退出
    }
    /* 若线程池中的任务没有满则继续增加任务到线程池给执行者 */
    pool->M[pool->queue_last].pools=pools;                              // 向任务队列的最后加入新的任务 
    pool->M[pool->queue_last].number=number;                            // 加入任务个数信息
    // 任务队列队尾序号最大不能超过任务队列容量（超过则取模来保证任务序号的顺序正确）
    pool->queue_last=(pool->queue_last+1)%(pool->queue_capacity);       // 队列循环处理
    pool->mission_num++;                                                // 任务总个数+1
    pthread_cond_signal(&pool->worker);                                 // 唤醒执行者来获取任务
    return;                 
}

/* 执行者线程 */
void* Worker(void* number) {
    POOL *pool=(POOL*)number;
    while(1) {
        //sleep(2);
        pthread_mutex_lock(&pool->mutex_pool);                      // 加入互斥锁对线程池进行保护
        /* 不停调用可用线程作为执行线程 */
        while(pool->mission_num==0&&!pool->close) {                 // 当前任务队列为空 并且 线程池没有关闭
            pthread_cond_wait(&pool->worker,&pool->mutex_pool);     // 执行者等待线程池解锁的信号  
            /* 关闭需要杀死的线程 */
            if(pool->kill!=0) {
                pool->kill--;                                       // 一次杀死一个线程
                if(pool->able>pool->min) {                          // 若当前可用线程（空闲）大于同时可以执行线程
                    pool->able--;                                   // 调用一个线程（让一个线程变得忙碌）
                    pthread_mutex_unlock(&pool->mutex_pool);        // 解锁线程池
                    Exit(pool);                                     // 退出线程池
                }
            }
        }
        // 若线程池关闭了则解锁后退出线程池
        if(pool->close) {
            pthread_mutex_unlock(&pool->mutex_pool);
            Exit(pool);
        }

        /* 每一个执行者线程从任务队列中取出任务 */
        mission mis;
        mis.pools=pool->M[pool->queue_head].pools;                      // 获取分配的任务函数
        mis.number=pool->M[pool->queue_head].number;                    // 获取分配的任务个数
        pool->queue_head=(pool->queue_head+1)%pool->queue_capacity;     // 对任务列表循环处理
        pool->mission_num--;                                            // 每从任务列表中取出一个需要处理的任务数就-1
        
        pthread_cond_signal(&pool->manager);                            // 唤醒管理者继续分配任务
        pthread_mutex_unlock(&pool->mutex_pool);                        // 解锁线程池从而开始执行单个执行者线程
        printf("pthread %ld start working!\n",pthread_self());          // 获取执行者进程ID(long long)

        /* 对执行者线程的特殊保护 */
        pthread_mutex_lock(&pool->mutex_run);
        pool->run++;                                                    // 增加一个在跑的线程
        pthread_mutex_unlock(&pool->mutex_run);
        
        /* 结束另一个运行线程 */
        mis.pools(mis.number);                                          // 传入任务个数number给pools函数并执行
        free(mis.number);                                               // 执行完后释放所有完成的任务
        mis.number=NULL;                                                // 任务个数置为0
        printf("pthread %ld end working!\n",pthread_self());            // 结束最后一个执行者线程并输出其tid

        pthread_mutex_lock(&pool->mutex_run);
        pool->run--;                                                    // 增加一个休眠线程
        pthread_mutex_unlock(&pool->mutex_run);
    }
    return pool;
}

/* 管理者线程 */
void* Manager(void* number) {
    POOL *pool=(POOL*)number;
    while(!pool->close) {                               // 线程池未关闭
        sleep(4);
        pthread_mutex_lock(&pool->mutex_pool);          // 对线程池加锁保护
        int missions_num=pool->mission_num;             // 获取任务个数
        int able=pool->able;                            // 获取可用线程个数
        pthread_mutex_unlock(&pool->mutex_pool);

        pthread_mutex_lock(&pool->mutex_run);
        int run=pool->run;                              // 获取正在执行线程个数
        pthread_mutex_unlock(&pool->mutex_run);

        // 创建线程
        if(((able-missions_num)<MIN)&&(able<pool->max)) {
        // 如果（可用线程数-任务总数 小于 同时可运行的执行者线程数）并且（ 可用线程 小于 最大执行者线程数）
            pthread_mutex_lock(&pool->mutex_pool);                              // 锁住线程池并向其中再加入任务
            int cnt=0;
            for(int i=0;i<pool->max&&cnt<NUM&&pool->able<pool->max;i++) {       // 不能超过最大需要杀死线程数
                if(pool->worker_pid[i]==0) {                                    // 若是休眠线程
                    pthread_create(&pool->worker_pid[i],NULL,Worker,pool);      // 创建一个可用的新线程作为执行者线程
                    pool->able++;                                               // 可接取任务的线程增加一条
                    cnt++;                                                      // 记录同时增加了几条执行者线程
                }
            }
            pthread_mutex_unlock(&pool->mutex_pool);                            // 停止向线程池中增加任务（解锁线程池）
        }
        // 销毁多余的线程（忙线程xNUM(需要销毁的线程数)小于可用线程，并且可用线程数大于最小线程数）
        else if(able>NUM*run&&able>pool->min) {
            pthread_mutex_lock(&pool->mutex_pool); 
            pool->kill=NUM;                                                     // 记录需要杀死的线程个数
            pthread_mutex_unlock(&pool->mutex_pool);
            for(int i=0;i<NUM;i++) {
                pthread_cond_signal(&pool->worker);                             // 阻塞需要杀死的线程直到需要它被唤醒
            }
        }
    }
    return NULL;
}

/* 获取当前在跑执行者线程数 */
int Pool_run(POOL* pool) {
    pthread_mutex_lock(&pool->mutex_run);
    int run_num=pool->run;
    pthread_mutex_unlock(&pool->mutex_run);
    return run_num;
}

/* 获取当前休眠（可用）线程数 */
int Pool_able(POOL* pool) {
    pthread_mutex_lock(&pool->mutex_pool);
    int able_num=pool->able;
    pthread_mutex_unlock(&pool->mutex_pool);
    return able_num;
}

/* 摧毁当前线程池 */
int Pool_destory(POOL* pool) {  
    if(pool==NULL) {                            // 如果线程池根本不存在则返回错误信息
        return -1;
    }
    pool->close=true;                           // 关闭线程池
    pthread_join(pool->manager_pid,NULL);       // 回收管理者
    for(int i=0;i<pool->able;i++) {
        pthread_cond_signal(&pool->worker);     // 回收执行者
    }

    /* 销毁所有互斥量和条件变量 */
    pthread_mutex_destroy(&pool->mutex_run);
    pthread_mutex_destroy(&pool->mutex_pool);
    pthread_cond_destroy(&pool->worker);
    pthread_cond_destroy(&pool->manager);
    if(pool) {
        free(pool);                             // 释放线程池空间
    }
    pool=NULL;
    return 0;                                   // 成功摧毁
}

/* 退出线程池 */
void Exit(POOL* pool) {
    pthread_t tid=pthread_self();
    for(int i=0;i<pool->max;i++) {                                  // 在线程池中的所有线程中进行寻找
        if(pool->worker_pid[i]==tid) {                              // 找到要销毁的线程tid
            pool->worker_pid[i]=0;                                  // 置为0（相当于将这个线程直接删除）
            printf("pthread %ld is exiting ....\n",tid);
        }
    }
    pthread_exit(NULL);                                             // 终止线程并且不返回任何数据
    return;
}

/* 执行者函数：获取当前运行线程的tid 和 此线程执行的任务个数 */
void pools(void *number) {
    int num=*(int *)number;                                         // 注意强制类型转换
    printf("pthread %ld is working ,ID = %d\n",pthread_self(),num); // long long
    sleep(2);
    return;
}

/* 主函数 */
int main() {
    POOL *pool=Create_Pool(MIN,MAX,MISSION);                    // 创建线程池
    for(int i=0;i<MISSION;i++) {
        int *num=(int *)malloc(sizeof(int));                    // 分配不同任务个数（个数递增）
        *num+=i;
        Pool_mission(pool,(void*)pools,(void*)num);             // 给不同的执行者线程（总共 MISSION 个任务，分配完则循环停止）
    }
    sleep(6);
    Pool_destory(pool);                                         // 完成所有任务后销毁线程池
    return 0;
}
