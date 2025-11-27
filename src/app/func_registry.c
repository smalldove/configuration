#include "func_public.h"
#include "func_registry.h"

// 声明各个模块的函数列表
extern struct func_attribute func_list_mode_1[10];
extern struct func_attribute func_list_mode_2[10];
extern struct func_attribute func_list_mode_3[10];

// 统一的函数注册表
struct func_attribute func_list[30] = {0};

// 初始化函数注册表
void init_func_registry(void)
{
    int index = 0;
    
    // 复制 mode_1 的函数
    for (int i = 0; i < 10 && func_list_mode_1[i].func != NULL; i++) {
        func_list[index++] = func_list_mode_1[i];
    }
    
    // 复制 mode_2 的函数
    for (int i = 0; i < 10 && func_list_mode_2[i].func != NULL; i++) {
        func_list[index++] = func_list_mode_2[i];
    }
    
    // 复制 mode_3 的函数
    for (int i = 0; i < 10 && func_list_mode_3[i].func != NULL; i++) {
        func_list[index++] = func_list_mode_3[i];
    }
}
