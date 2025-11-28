#ifndef FUNC_REGISTRY_H
#define FUNC_REGISTRY_H

#include "func_public.h"

// 函数注册宏定义
#define REGISTER_FUNC_START(module_name) \
    static struct func_attribute func_list_##module_name = {

#define REGISTER_FUNC(id, in_func, main_func, out_func) \
        .input_num = 1, \
        .output_num = 1, \
        .id_func = id, \
        .arg = NULL, \
        .in_attribute = in_func, \
        .func = main_func, \
        .out_attribute = out_func \
    

#define REGISTER_FUNC_END(module_name) \
    }; \
    static void __attribute__((constructor)) register_##module_name(void) { \
        register_module_functions(&func_list_##module_name); \
    }

// 模块注册函数
void register_module_functions(struct func_attribute *funcs);

// 根据函数ID查找函数
struct func_attribute *find_function_by_id(const char *func_id);

// 初始化函数注册表
void init_func_registry(void);

// 获取注册的函数数量
int get_registered_func_count(void);

// 遍历所有注册的函数（用于调试和显示）
void iterate_all_functions(void (*callback)(struct func_attribute *func, void *arg), void *arg);

// 清理平衡树（程序退出时调用）
void cleanup_func_registry(void);

// 平衡树性能测试专用函数 - 获取树的高度信息
int get_tree_height_info(int *max_height);

#endif
