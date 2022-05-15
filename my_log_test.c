#include "mylog.h"
int main(void) {
    my_log_printf(MY_LOG_DEBUG, true,           //  什么日志等级，是否需要打印，打印内容
    "\n**********************************TEST START**********************************");
    int number = 0;
    while (number!=4) {
        my_log_printf(MY_LOG_DEBUG, true, "number=%d", number);
        number++;
        sleep(1);
    }
    return 0;
}
