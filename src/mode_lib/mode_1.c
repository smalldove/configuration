#include "func_public.h"
#include "func_registry.h"
#include <stdio.h>
#include <stdlib.h>

/*

定义输入参数一个、输出参数一个
input:
    0.  type: int
    深拷贝

output:
    0.  type: int

arg: int
*/

static int func_1_in(struct my_node *self, void *arg, int arg_n)
{
    printf("func_1_in 执行\n");

    return 0;
    switch (arg_n) { 
        case 0: 
            ((int*)self->self->arg)[0] = *(int*)arg;
        break;
        case 1: 
            ((int*)self->self->arg)[1] = *(int*)arg;
        default:
        break;
    }

    return 0;
}

static int func_1(struct my_node *self)
{
    if(NULL == self && NULL == self->self->arg)
    {
        return -1;
    }
    *(int*)(self->self->arg) += 2;
    printf("func_1 执行\n");
    return 0;
}

static void *func_1_out(struct my_node *self, int arg_n)
{
    printf("func_1_out 执行\n");
    return NULL;
    switch (arg_n) { 
        case 0: 
            return &((int*)self->self->arg)[0];
        break;
        case 1: 
            return &((int*)self->self->arg)[1];
        default:
        break;
    }
    return NULL;
}

struct func_attribute *init_mode_1(void)
{
    struct func_attribute *self = calloc(sizeof(struct func_attribute), 1);
    self->arg = calloc(sizeof(int), 2);
    self->func = func_1;

    self->in_attribute = func_1_in;
    self->input_num = 1;

    self->out_attribute = func_1_out;
    self->output_num = 1;

    return self;
}

REG_MODULE_FUNC(init_mode_1, mode_1)
