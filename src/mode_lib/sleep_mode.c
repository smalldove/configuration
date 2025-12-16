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

static int func_sleep_in(struct my_node *self, void *arg, int arg_n)
{
    return 0;
}

static int func_sleep(struct my_node *self)
{
    printf("⭐️:id: %s 延时程序执行\n", self->id_name);
    sleep(1);
    return 0;
}

static void *func_sleep_out(struct my_node *self, int arg_n)
{
    return NULL;
}

struct func_attribute *init_sleep_mode(void)
{
    struct func_attribute *self = calloc(sizeof(struct func_attribute), 1);
    self->arg = calloc(sizeof(int), 2);
    self->func = func_sleep;

    self->in_attribute = func_sleep_in;
    self->input_num = 1;

    self->out_attribute = func_sleep_out;
    self->output_num = 1;

    return self;
}

REG_MODULE_FUNC(init_sleep_mode, sleep_mode)
