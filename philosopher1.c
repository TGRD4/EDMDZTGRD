/*
1.至多允许有四位哲学家同时去拿左边的筷子，保证至少有一位哲学家能够进餐，并在用餐完毕后释放他占用的筷子，从而使别的哲学家进餐
2.规定奇数号哲学家先拿左边的筷子，再拿右边的筷子，偶数号哲学家则相反
3.仅当哲学家的左右两支筷子可用时，才允许他拿起筷子
*/

/* 1 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
/*
Semaphore 信号量，可以用来控制同时访问特定资源的线程数量，通过协调各个线程，以保证合理的使用资源
通常用于那些资源有明确访问数量限制的场景，常用于限流
*/
sem_t cnt;              // long int
sem_t chopsticks[5];    // char
pthread_mutex_t mutex;
void *eat(int number) {
    int l=0,r=0;
    l=number;
    r=((number==4)?0:number+1);     //存在特殊情况，第五只筷子作为左筷子时，右筷子又变成了第一只筷子（序号为1(圆)）
    while(1) {
        sleep(2);                    //哲学家正在思考
        sem_wait(&cnt);   
/*
函数sem_wait( sem_t *sem )被用来阻塞当前线程直到信号量sem的值大于0，解除阻塞后将sem的值减一，表明公共资源经使用后减少
*/
        /* 判断左边筷子是否可用 *///pthread_mutex_lock(&mutex);
        sem_wait(&chopsticks[l]);
        printf("philosopher %d got the chopstick %d!\n",number+1,l+1);      // 编号+1
        /* 判断右边筷子是否可用 */
        sem_wait(&chopsticks[r]);
        printf("philosopher %d got the chopstick %d!\n",number+1,r+1);

        /* 吃 */
        printf("philosopher %d is eating!\n",number+1);
        /* 吃完休息 */
        sleep(4);
        /* 释放左右筷子到原位 */
        sem_post(&chopsticks[l]);       // 信号量+1（恢复左右筷子的使用权）
        printf("philosopher %d put the chopstick %d!\n",number+1,l+1);
        sem_post(&chopsticks[r]);   
        printf("philosopher %d put the chopstick %d!\n",number+1,r+1);
        sem_post(&cnt);                 // 不恢复信号值则会造成有人吃不上饭（一直堵塞）
/*
函数sem_post( sem_t *sem )用来增加信号量的值
当有线程阻塞在这个信号量上时，调用这个函数会使其中的一个线程不在阻塞，选择机制同样是由线程的调度策略决定的
*/
    }
}

int main() {
    pthread_t phi[5];
    pthread_mutex_init(&mutex,NULL);
    sem_init(&cnt,0,4);         
    //cnt为指向信号量的一个指针，0为在当前进程中的所有线程共享，并设置初始信号量cnt为4(最多只允许四位哲学家同时存在拿筷子的动作)
    for(int i=0;i<5;i++) {              //五次循环保证五位哲学家都能吃上饭
        sem_init(&chopsticks[i],0,1);   //初始化信号量chopsticks为1（五只筷子每一轮最开始都在桌子上）
        pthread_create(&phi[i],NULL,(void*)eat,(void*)i);   //为每一个哲学家创建一次线程
    }
    for(int i=0;i<5;i++) {
        pthread_join(phi[i],NULL);      //一直等待五个线程的返回值
    }
    /* 当 pthread_join() 函数返回后，被调用线程才算真正意义上的结束，它的内存空间也会被释放（如果被调用线程是非分离的）
    被释放的内存空间仅仅是系统空间，你必须手动清除程序分配的空间，比如 malloc() 分配的空间 */
    pthread_mutex_destroy(&mutex);
    return 0;
}
