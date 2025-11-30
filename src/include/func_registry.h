#ifndef FUNC_REGISTRY_H
#define FUNC_REGISTRY_H

#include "func_public.h"

#define REG_MODULE_FUNC(func, id)   \
static void __attribute__((constructor)) register_##id(void) {\
    register_module_functions(func, #id);\
}
typedef struct func_attribute *(*REG_MODE_FUNC_T)(void);
// 平衡树字典结构 - 零碰撞确定性结构

typedef struct tree_node {
    char *key;                    // 函数ID字符串
    REG_MODE_FUNC_T func;  // 函数属性指针
} tree_node_t;


// 模块注册函数
void register_module_functions(struct func_attribute (*(*funcs)(void)), char *func_id);

// 根据函数ID查找函数
REG_MODE_FUNC_T find_function_by_id(const char *func_id);

// 获取注册的函数数量
int get_registered_func_count(void);

// 遍历所有注册的函数（用于调试和显示）
void iterate_all_functions(void (*callback)(REG_MODE_FUNC_T func, void *arg), void *arg);

// 清理平衡树（程序退出时调用）
void cleanup_func_registry(void);

// 平衡树性能测试专用函数 - 获取树的高度信息
int get_tree_height_info(int *max_height);

#endif
