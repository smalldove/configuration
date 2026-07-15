#define _POSIX_C_SOURCE 200809L
#include "ck_queue.h"
#include "list.h"
#include <cjson/cJSON.h>
#include "actuator.h"
#include "func_public.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "list_cfg_loading.h"
// 初始化执行器模块
struct actuator_thread_t *actuator_ini(int production_num, void *(*production_unit)(void *), int execution_num, void *(*execution_unit)(void *), void *arg)
{
    struct actuator_thread_t *act = calloc(sizeof(struct actuator_thread_t), 1);

    if(NULL == act)
    {
        return NULL;
    }


    act->production_num = production_num;
    act->production_unit = production_unit;
    act->execution_num = execution_num;
    act->execution_unit = execution_unit;
    act->arg = arg;
    
    return act;
}

// 创建线程
int actuator_create(struct actuator_thread_t *act)
{
    if (act == NULL ) {
        fprintf(stderr, "Invalid arguments for actuator_create\n");
        return -1;
    }

    
    struct CK_HEAD_t *ck_queue = calloc(sizeof(struct CK_HEAD_t), 1);
    if (ck_queue == NULL) {
        fprintf(stderr, "Failed to allocate CK queue\n");
        return -1;
    }
    act->ck_queue = ck_queue;
    CK_STAILQ_INIT(&ck_queue->production_head);
    CK_STAILQ_INIT(&ck_queue->execution_head);

    for (int i = 0; i < act->production_num; i++)
    {
        act->production_id[i] = 0;
        struct actuator_pro_arg_t *arg = calloc(sizeof(struct actuator_pro_arg_t ), 1);
        act->production_arg[i] = arg;
        arg->cnt = i;
        arg->arg = act->arg;
        arg->ck_queue = ck_queue;

        int ret = pthread_create(&act->production_id[i], NULL, act->production_unit, arg);
        if (ret != 0) {
            fprintf(stderr, "Failed to create production thread %d: %d\n", i, ret);
            return -1;
        }
    }
    for (int i = 0; i < act->execution_num; i++)
    {
        act->execution_id[i] = 0;
        struct actuator_exec_arg_t *arg = calloc(sizeof(struct actuator_exec_arg_t), 1);
        act->execution_arg[i] = arg;
        arg->cnt = i;
        arg->arg = act->arg;
        arg->ck_queue = ck_queue;

        int ret = pthread_create(&act->execution_id[i], NULL, act->execution_unit, arg);

        if (ret != 0) {
            fprintf(stderr, "Failed to create execution thread %d: %d\n", i, ret);
            return -1;
        }
    }

    return 0;
}

/* 销毁执行器，取消所有线程并释放资源 */
int actuator_destroy(struct actuator_thread_t *act)
{
    if (act == NULL) {
        return -1;
    }

    /* 取消所有生产线程 */
    for (int i = 0; i < act->production_num; i++) {
        if (act->production_id[i] != 0) {
            pthread_cancel(act->production_id[i]);
        }
    }

    /* 取消所有执行线程 */
    for (int i = 0; i < act->execution_num; i++) {
        if (act->execution_id[i] != 0) {
            pthread_cancel(act->execution_id[i]);
        }
    }

    /* 等待所有线程退出 */
    for (int i = 0; i < act->production_num; i++) {
        if (act->production_id[i] != 0) {
            pthread_join(act->production_id[i], NULL);
        }
    }
    for (int i = 0; i < act->execution_num; i++) {
        if (act->execution_id[i] != 0) {
            pthread_join(act->execution_id[i], NULL);
        }
    }

    /* 清理队列中残留的缓冲区 */
    if (act->ck_queue != NULL) {
        struct pro_buf *pb, *pb_tmp;
        struct exec_buf *eb, *eb_tmp;

        CK_STAILQ_FOREACH_SAFE(pb, &act->ck_queue->production_head, queue_pro, pb_tmp) {
            CK_STAILQ_REMOVE(&act->ck_queue->production_head, pb, pro_buf, queue_pro);
            free(pb);
        }
        CK_STAILQ_FOREACH_SAFE(eb, &act->ck_queue->execution_head, queue_exec, eb_tmp) {
            CK_STAILQ_REMOVE(&act->ck_queue->execution_head, eb, exec_buf, queue_exec);
            free(eb);
        }

        free(act->ck_queue);
    }

    /* 释放各线程参数分配的内存 */
    for (int i = 0; i < act->production_num; i++) {
        free(act->production_arg[i]);
    }
    for (int i = 0; i < act->execution_num; i++) {
        free(act->execution_arg[i]);
    }

    free(act);
    return 0;
}

void get_start(struct my_node *node, void *arg)
{
    if(NODE_START == node->type)
    {
        struct CK_HEAD_t *ck_queue = arg;
        struct pro_buf *buf = calloc(sizeof(struct pro_buf), 1);

        atomic_store(&buf->pro, node);
        CK_STAILQ_INSERT_TAIL(&ck_queue->production_head, buf, queue_pro);
    }
    return;
}

/*驱动器-生产分配*/
void *actuator_production(void *arg)
{
    if(NULL == arg)
    {
        fprintf(stderr, "Error: NULL argument, cannot execute\n");
        return NULL;
    }

    
    struct exec_buf *tmp = NULL;
    struct pro_buf *queues = NULL;
    struct actuator_pro_arg_t *act_arg = arg;
    struct CK_HEAD_t *ck_queue = act_arg->ck_queue;

    iterate_all_cfg_nodes(get_start, ck_queue);

    while (1) {
        /*线程优化*/
        sched_yield();
        pthread_testcancel();
        /*
        工作流说明：数据缓冲区放置待执行的功能单元，遍历可执行队列，如有可调用的执行单元，将功能单元从缓冲区取出，置于执行单元缓冲
        */

        // 检查队列是否为空
        if (!CK_STAILQ_EMPTY(&ck_queue->production_head)) {
            if(!CK_STAILQ_EMPTY(&ck_queue->execution_head))
            {
                tmp = CK_STAILQ_FIRST(&ck_queue->execution_head);
                // 获取队首元素
                queues = CK_STAILQ_FIRST(&ck_queue->production_head);
                if (queues != NULL) {
                    // 从队列中移除队首元素

                    CK_STAILQ_REMOVE(&ck_queue->execution_head, tmp, exec_buf, queue_exec);
                    CK_STAILQ_REMOVE(&ck_queue->production_head, queues, pro_buf, queue_pro);
                    atomic_store(&tmp->exec, atomic_load(&queues->pro));

                    // 原子写入：将生产队列中的节点转移到执行缓冲区
                    free(queues);
                }

            }
            else 
            {
            
            }
            
        }
    }
    return NULL;
}

/*驱动器-执行*/
void *actuator_execution(void *arg)
{
    if(NULL == arg)
    {
        fprintf(stderr, "Error: NULL argument, cannot execute\n");
        return NULL;
    }

    struct actuator_exec_arg_t *act_arg = arg;
    struct my_node *self_node = NULL;
    struct exec_buf *tmp = calloc(sizeof(struct exec_buf), 1);
    struct CK_HEAD_t *ck_queue = act_arg->ck_queue;

    CK_STAILQ_INSERT_TAIL(&ck_queue->execution_head, tmp, queue_exec);

    while (1) {

        /*线程优化*/

        sched_yield();
        pthread_testcancel();

        // 正确：使用原子存储
        // atomic_store(&tmp->exec, queue);

        // // 正确：使用原子加载
        // struct my_node *current = atomic_load(&tmp->exec);

        // // 正确：原子交换
        // struct my_node *old = atomic_exchange(&tmp->exec, new_ptr);
        
        self_node = atomic_load(&tmp->exec);
        if(self_node)
        {
            // 执行节点函数
            if(self_node->self && self_node->self->func)
            {
                self_node->self->func(self_node);
            }

            if (self_node->out_list && self_node->out_list->num)
            {
                for(int i = 0; i < self_node->out_list->num; i++)
                {
                    self_node->out_list->node[i]->self->in_attribute(self_node->out_list->node[i], self_node->self->out_attribute(self_node, self_node->out_list->arg_out[i]), self_node->out_list->arg_in[i]);
                }

            } 
            else 
            {
                // printf("%s无输出链路\n", self_node->id_name);
            }

            for (int i = 0; i < LIST_MAX; i++) 
            {
                if(self_node->next[i] == NULL)
                    break;
                struct pro_buf *pro_back = calloc(sizeof(struct pro_buf),1);
                if (pro_back != NULL) {
                    // 将下一个节点放入生产缓冲区
                    atomic_store(&pro_back->pro, self_node->next[i]);
                    CK_STAILQ_INSERT_TAIL(&ck_queue->production_head, pro_back, queue_pro);
                }
            }

            atomic_store(&tmp->exec, NULL); // 清空执行指针
            CK_STAILQ_INSERT_TAIL(&ck_queue->execution_head, tmp, queue_exec);

        }
    }
    return NULL;
}
