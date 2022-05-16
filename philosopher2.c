/* 2 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t chopsticks[5];    
pthread_mutex_t mutex;
void *eat(int number) {
    int l=0,r=0;
    l=number;
    r=((number==4)?0:number+1);     
    while(1) {
        sleep(2);       
        
        //规定奇数号哲学家先拿左边的筷子，再去拿右边的筷子
        if((number+1)%2==1) {
            sem_wait(&chopsticks[l]);
            printf("philosopher %d got the chopstick %d!\n",number+1,l+1);     
            sem_wait(&chopsticks[r]);
            printf("philosopher %d got the chopstick %d!\n",number+1,r+1);
        }
        //偶数相反
        else {
            sem_wait(&chopsticks[r]);
            printf("philosopher %d got the chopstick %d!\n",number+1,r+1);     
            sem_wait(&chopsticks[l]);
            printf("philosopher %d got the chopstick %d!\n",number+1,l+1);
        }
        printf("philosopher %d is eating!\n",number+1);
        sleep(4);
        
        sem_post(&chopsticks[l]);      
        printf("philosopher %d put the chopstick %d!\n",number+1,l+1);
        sem_post(&chopsticks[r]);   
        printf("philosopher %d put the chopstick %d!\n",number+1,r+1);
    }
}

int main() {
    pthread_t phi[5];
    pthread_mutex_init(&mutex,NULL);       
    for(int i=0;i<5;i++) {           
        sem_init(&chopsticks[i],0,1);   
        pthread_create(&phi[i],NULL,(void*)eat,(void*)i);   
    }
    for(int i=0;i<5;i++) {
        pthread_join(phi[i],NULL);   
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}
