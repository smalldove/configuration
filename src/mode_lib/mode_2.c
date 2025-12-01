#include "func_public.h"
#include "func_registry.h"
/*

input:
    0.  type: int
    深拷贝

output:
    0.  type: int

arg: int
*/

static int func_2_in(struct my_node *self, void *arg, int arg_n)
{
    if(NULL == arg)
    {
        return 1;
    }
    switch (arg_n) { 
        case 0: 
            ((int*)(self->self->arg))[0] = *(int*)arg;
        break;
            ((int*)(self->self->arg))[1] = *(int*)arg;
        default:
        break;
    }

    return 0;
}

static int func_2(struct my_node *self)
{
    if(NULL == self || NULL == self->self->arg)
    {
        return 0;
    }
    
    // 重置路径
    self->next[0] = self->next_table[((int*)self->self->arg)[0] > ((int*)self->self->arg)[0] ? 0 : 1];

    return 0;
}

static void *func_2_out(struct my_node *self, int arg_n)
{
    switch (arg_n) { 
        case 0: 
            return &((int *)self->self->arg)[2];
        break;
        default:
        break;
    }
    return NULL;
}


struct func_attribute *init_mode_2(void)
{
    struct func_attribute *self = calloc(sizeof(struct my_node), 1);
    self->arg = calloc(sizeof(int), 3);
    self->func = func_2;

    self->in_attribute = func_2_in;
    self->input_num = 2;

    self->out_attribute = func_2_out;
    self->output_num = 1;

    return self;
}


REG_MODULE_FUNC(init_mode_2, mode_2)
