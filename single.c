/* 单个生产者和消费者 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
struct Msg                  //共享数据
{
    int num;                //食物个数
    struct Msg *next;      
};

struct Msg *head;           //公共区域的食物
 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;      //定义并初始化一个互斥量(代替init)
pthread_cond_t has_data = PTHREAD_COND_INITIALIZER;     //定义并初始化一个条件变量
 
void err_thread(int ret,char *str)      //如果线程创建失败，则返回失败的原因，并退出线程
{
    if(ret != 0) {
        fprintf(stderr,"%s:%s\n",str,strerror(ret));    //将str字符串内容输入进stderr流中//strerror返回错误ret编号代表的信息
        pthread_exit(NULL);             // NULL作为线程退出时的返回值
    }
}

void *produser(void *arg)    //生产者
{
    while(1) {
        struct Msg *food = (struct Msg *)malloc(sizeof(struct Msg));    //生产一个食物
        food->num = rand() % 1000 + 1;              //初始化食物（），模拟生产了一个数据
        printf("--produser %d\n",food->num);        //生产者产出的食物
        pthread_mutex_lock(&mutex);                 //加锁（进入写入公共区域的线程）
        food->next = head;                          //写入公共区域      
        head = food;                                
        pthread_mutex_unlock(&mutex);               //解锁

        /*告诉消费者可以开启线程（购买食物）*/
        pthread_cond_signal(&has_data);              //唤醒阻塞在条件变量 has_data上 的线程（唤醒消费者结束等待来进行消费）
/*
pthread_cond_signal函数的作用是发送一个信号给另外一个正在处于阻塞等待状态的线程,使其脱离阻塞状态,继续执行
如果没有线程处在阻塞等待状态,pthread_cond_signal也会成功返回
*/
        sleep(1);                           //随机沉睡几秒
    }
}
 
void *consumer(void *arg)    //消费者
{
    struct Msg *food;
    while(1) {                                          //一直消费食物

        pthread_mutex_lock(&mutex);                     //加锁 互斥量
        if(head == NULL) {                              //若公共区域不存在食物则等待生产者生成食物
            pthread_cond_wait(&has_data,&mutex);        //阻塞当前线程 等待has_data条件变量pthread_cond_signal()的唤醒（等待生产）
/*
pthread_cond_wait()用于阻塞当前线程，等待别的线程使用 pthread_cond_signal()或pthread_cond_broadcast() 来唤醒它
pthread_cond_wait()必须与 pthread_mutex 配套使用
pthread_cond_wait()函数一进入wait状态就会自动release mutex,当其他线程通过pthread_cond_signal()/pthread_cond_broadcast()把该线程唤醒，
使pthread_cond_wait()通过(返回)时，该线程又自动获得该mutex
*/
        }                                               //pthread_cond_wait 返回时(生产者生产完毕)，重新加锁（开始消费）                       
        food = head;
        head = food->next;
        pthread_mutex_unlock(&mutex);                   //解锁

        printf("------------------consumer:%d\n",food->num);
        free(food);                                     //释放food结构体（消费完毕）
        sleep(1);
    }
}
 
int main() {
    int ret;
    /*
    pthread_mutex_init(&mutex, NULL);       //初始化互斥锁
    pthread_cond_init(&has_data, NULL);     //初始化条件变量
    */
    pthread_t pid,pidd;
    srand(time(NULL));                                  //初始化随机数种子为当前时间
    ret=pthread_create(&pid,NULL,produser,NULL);        //生产者线程（成功创建返回0）（线程属性和函数实参都默认设置为NULL）
    if(ret != 0) {
        err_thread(ret,"pthread_creat error");          //ret返回给err_thread错误编号和需要输出的错误信息字符串
    }
    ret=pthread_create(&pidd,NULL,consumer,NULL);        //消费者线程
    if(ret != 0) {
        err_thread(ret,"pthread_creat error");
    }
    pthread_join(pid,NULL);
    pthread_join(pidd,NULL);

/*
    pthread_mutex_destroy(&mutex);                   //销毁锁
    pthread_cond_destroy(&has_data);                 //销毁条件变量
*/
/*
pthread_mutex_destroy()用于注销一个互斥锁，API定义如下： int pthread_mutex_destroy(pthread_mutex_t *mutex)
销毁一个互斥锁即意味着释放它所占用的资源，且要求锁当前处于开放状态。
由于在Linux中，互斥锁并不占用任何资源，因此LinuxThreads中的pthread_mutex_destroy()除了检查锁状态以外（锁定状态则返回EBUSY）没有其他动作。

注意：pthread_cond_destroy()释放条件变量，但条件变量占用的空间并未被释放。
*/
    return 0;
}

/*
获取某个线程执行结束时返回的数据：
int pthread_join(pthread_t thread, void ** retval);

thread 参数用于指定接收哪个线程的返回值；retval 参数表示接收到的返回值，
如果 thread 线程没有返回值，又或者我们不需要接收 thread 线程的返回值，可以将 retval 参数置为 NULL。

pthread_join() 函数会一直阻塞调用它的线程，直至目标线程执行结束（接收到目标线程的返回值），阻塞状态才会解除。
如果 pthread_join() 函数成功等到了目标线程执行结束（成功获取到目标线程的返回值），返回值为数字 0；

反之如果执行失败，函数会根据失败原因返回相应的非零值，每个非零值都对应着不同的宏，例如：
EDEADLK：检测到线程发生了死锁。
EINVAL：分为两种情况，要么目标线程本身不允许其它线程获取它的返回值，要么事先就已经有线程调用 pthread_join() 函数获取到了目标线程的返回值。
ESRCH：找不到指定的 thread 线程。
以上这些宏都声明在 <errno.h> 头文件中，如果程序中想使用这些宏，需提前引入此头文件。

再次强调，一个线程执行结束的返回值只能由一个 pthread_join() 函数获取，当有多个线程调用 pthread_join() 函数获取同一个线程的执行结果时，
哪个线程最先执行 pthread_join() 函数，执行结果就由那个线程获得，其它线程的 pthread_join() 函数都将执行失败。
*/
