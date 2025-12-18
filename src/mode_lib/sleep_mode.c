#include "func_public.h"
#include "func_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct time_t_ctrl
{
    time_t new;
    int state;
    int sleep_next;
    int sleep_total;
};
/*

定义输入参数一个、输出参数一个
input:
    0.  type: int
    深拷贝

output:
    0.  type: int

arg: int
*/

static int func_sleep_in(struct my_node *self, void *arg, int arg_n)
{
    return 0;
}

static int func_sleep(struct my_node *self)
{
    struct timespec *last_time = self->self->arg;
    struct timespec current_time;

    // 获取当前时间
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    // 如果是第一次调用，初始化 last_time
    if (last_time->tv_sec == 0 && last_time->tv_nsec == 0) {
        last_time->tv_nsec = current_time.tv_nsec;
        last_time->tv_sec = current_time.tv_sec;
        self->next[0] = self;
        self->next[1] = NULL;

        return 0;
    }

    // 计算时间差（单位：秒）
    double elapsed = (current_time.tv_sec - last_time->tv_sec) +
                     (current_time.tv_nsec - last_time->tv_nsec) / 1e9;

    if (elapsed >= 1.0) {
        printf("⭐️: id:%s  延时1s  执行时间戳:%ld\n", self->id_name, time(NULL));
        last_time->tv_nsec = current_time.tv_nsec;
        last_time->tv_sec = current_time.tv_sec;
        
        self->next[0] = self->next_table[0];
        self->next[1] = self->next_table[1];

    } else {
        // 未到时间，延时最多1ms，降低CPU占用
        usleep(1000);  // 1000微秒 = 1毫秒
        self->next[0] = self;
        self->next[1] = NULL;
    }

    return 0;
}

static void *func_sleep_out(struct my_node *self, int arg_n)
{
    return NULL;
}

struct func_attribute *init_sleep_mode(void)
{
    struct func_attribute *self = calloc(sizeof(struct func_attribute), 1);
     
    struct timespec *arg = calloc(sizeof(struct timespec ), 1);
    arg->tv_nsec = 0;
    arg->tv_sec = 0;

    self->arg = arg;
    self->func = func_sleep;

    self->in_attribute = func_sleep_in;
    self->input_num = 1;

    self->out_attribute = func_sleep_out;
    self->output_num = 1;

    return self;
}

REG_MODULE_FUNC(init_sleep_mode, sleep_mode)
