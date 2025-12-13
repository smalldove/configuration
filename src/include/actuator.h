#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "func_public.h"  
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>


struct actuator_thread_t
{
    pthread_t production_id[32];
    int production_num;

    pthread_t execution_id[32];
    int execution_num;

    void *production_arg;
    void *(*production_unit)(void *);
    
    void *execution_arg;
    void *(*execution_unit)(void *);

    int exec_cnt;   // 执行者计数
};



// 生产者的缓冲区
struct actuator_pro_arg_t
{
    CK_LIST_HEAD(pro_list, my_node) production_head;
};

// 执行者可调用队列
struct actuator_exec_arg_t
{
    struct exec_buf
    {
        CK_LIST_ENTRY(exec_buf) queue;
        _Atomic(struct my_node *) exec;// 原子操作
    }buffer[32];
    
    CK_LIST_HEAD(exec_list, exec_buf) execution_head;
    /*
    execution_head 的作用，在执行线程处于无任务执行状态时，向队列中提交自己的缓冲区
    而生产者检查队列是否有空闲的执行者，有，则先将执行者缓冲区出队，将待执行单元移交在执行者的缓冲区中
    而执行者负责检查自己缓冲区，如果有数据传递，则执行，执行后，再将自己的缓冲区移交队列
    以此循环执行
    此方案可以将多个起始点并行执行，且执行单元与执行链路相互解耦，提高执行效率
    且该队列也可方便多生产者的实现 但目前先不考虑多生产者场景 即性能瓶颈受限生产者单元处理能力
    */
    
};

struct actuator_thread_t *actuator_ini(int production_num, void *(*production_unit)(void *), void *production_arg, int execution_num, void *(*execution_unit)(void *), void *execution_arg);
int actuator_create(struct actuator_thread_t *act);

#endif
