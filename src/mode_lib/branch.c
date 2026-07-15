#include "func_public.h"
#include "func_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*

定义输入参数一个、输出参数一个
input:
    0.  type: int
    深拷贝

output:
    0.  type: int

arg: int
*/

static int func_branch_in(struct my_node *self, void *arg, int arg_n)
{
    (void)self;
    (void)arg;
    (void)arg_n;
    return 0;
}

static int func_branch(struct my_node *self)
{
    for(int i = 0; i < LIST_MAX; i++)
    {
        self->next[i] = NULL;
    }

    self->next[0] = self->next_table[1];

    printf("🔶:id: %s 分支选择执行、现在先写死只选择第二个输出链路 下一个节点:id %s\n", self->id_name, self->next[0]->id_name);
    return 0;
}

static void *func_branch_out(struct my_node *self, int arg_n)
{
    (void)self;
    (void)arg_n;
    return NULL;
}

struct func_attribute *init_branch_mode(void)
{
    struct func_attribute *self = calloc(sizeof(struct func_attribute), 1);
    self->arg = calloc(sizeof(int), 2);
    self->func = func_branch;

    self->in_attribute = func_branch_in;
    self->input_num = 1;

    self->out_attribute = func_branch_out;
    self->output_num = 1;

    return self;
}

REG_MODULE_FUNC(init_branch_mode, branch_mode)
