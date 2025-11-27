#include "func_public.h"
#include "func_registry.h"

static int func_2_in(struct my_node *self, void *arg, int *arg_n)
{
    
    return 0;
}

static int func_2(struct my_node *self, void *arg, int arg_n)
{
    printf("func_2 执行");
    return 0;
}

static void *func_2_out(struct my_node *self, int arg_n)
{
    return 0;
}

// 优雅的函数注册
REGISTER_FUNC_START(mode_2)
    REGISTER_FUNC("func_2", func_2_in, func_2, func_2_out)
REGISTER_FUNC_END()
