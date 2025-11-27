#ifndef FUNC_REGISTRY_H
#define FUNC_REGISTRY_H

#include "func_public.h"

// 函数注册宏定义
#define REGISTER_FUNC_START(module_name) \
    struct func_attribute func_list_##module_name[10] = {

#define REGISTER_FUNC(id, in_func, main_func, out_func) \
    { \
        .input_num = 1, \
        .output_num = 1, \
        .id_func = id, \
        .arg = NULL, \
        .in_attribute = in_func, \
        .func = main_func, \
        .out_attribute = out_func \
    },

#define REGISTER_FUNC_END() \
    };

// 初始化函数注册表
void init_func_registry(void);

#endif
