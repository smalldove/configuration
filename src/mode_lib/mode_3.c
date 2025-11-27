#include "func_public.h"
#include "func_registry.h"

static int func_3_in(struct my_node *self, void *arg, int *arg_n)
{
    
    return 0;
}

static int func_3(struct my_node *self, void *arg, int arg_n)
{
    printf("func_3 执行");
    return 0;
}

static void *func_3_out(struct my_node *self, int arg_n)
{
    return 0;
}

// 优雅的函数注册
REGISTER_FUNC_START(mode_3)
    REGISTER_FUNC("func_3", func_3_in, func_3, func_3_out)
REGISTER_FUNC_END()
