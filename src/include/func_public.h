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
    NODE_SWITCH,
    NODE_NON
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
    void *arg;      
    /*
    self : 自身块指针
    arg : 入参
    arg_n : 指定输入号(从0开始)
    */
    int (*in_attribute)(struct my_node *self, void *arg, int arg_n);
    
    /*
    函数执行
    */
    
    int (*func)(struct my_node *self);
    /*
    输出参数回调
    self : 自身块指针
    arg_n : 指定输出号(从0开始)
    */
    void *(*out_attribute)(struct my_node *self, int arg_n);

};

#define NODE_ID 64
#define LIST_MAX 64

struct out_list
{
    struct list_head out_list;      // 对方链表地址
    int arg_n;                      // 对方输出指向
};

struct my_node {

    char id_name[NODE_ID];                   // 实例名称
    
    enum Node_type type;

    struct list_head link;          // 本节点引用点
    struct list_head next[LIST_MAX];          // 链路的指向调用表
    /*默认限制是最大32个并发链路，可拓展，默认初始化时next与next_table保持一致*/
    struct list_head next_table[LIST_MAX];    // 链路的指向实例总表
    
    struct out_list out_list[LIST_MAX];          // 输出指向的调用表
    struct out_list out_list_table[LIST_MAX];    // 输出指向的实例总表
    
    struct func_attribute *self;
};

#endif
