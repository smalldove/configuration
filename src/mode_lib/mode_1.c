#include "func_public.h"
#include "func_registry.h"

static int func_1_in(struct my_node *self, void *arg, int *arg_n)
{

    return 0;
}

static int func_1(struct my_node *self, void *arg, int arg_n)
{
    printf("func_1 执行");
    return 0;
}

static void *func_1_out(struct my_node *self, int arg_n)
{
    return 0;
}

// 优雅的函数注册
REGISTER_FUNC_START(mode_1)
    REGISTER_FUNC("func_1", func_1_in, func_1, func_1_out)
REGISTER_FUNC_END(mode_1)