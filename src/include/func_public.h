#ifndef FUNC_PUBLIC_H
#define FUNC_PUBLIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

enum Node_type
{
    NODE_START = 0,
    NODE_END,
    NODE_RELAY,
    NODE_FANSHAPED,
    NODE_SWITCH
};


enum node_status
{
    NODE_NO_CALL,
    NODE_AWAIT,
    NODE_RUN,
};

struct my_node;

struct func_attribute
{
    int input_num;      // 支持输入的个数
    int output_num;     // 支持输出的个数
    char id_func[128];  // id追溯唯一判定 例如哈希值

    void *arg;      
    
    /*
    self : 自身块指针
    arg : 入参
    arg_n : 指定输入号(从0开始)
    */
    int (*in_attribute)(struct my_node *self, void *arg, int *arg_n);
    /*
    函数执行
    */
    
    int (*func)(struct my_node *self, void *arg, int arg_n);
    /*
    输出参数回调
    self : 自身块指针
    arg_n : 指定输出号(从0开始)
    */
    void *(*out_attribute)(struct my_node *self, int arg_n);

};

struct my_node {

    char id_name[64];               // 实例名称
    
    enum Node_type type;
    struct list_head list[32];      // 内嵌链表节点
    int num;

    struct list_head out_list[32];  // 输出指向
    int out_num;
    
    struct func_attribute self;
};

extern struct func_attribute func_list[30];

#endif
