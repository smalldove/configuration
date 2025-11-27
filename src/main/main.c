#include "func_public.h"
#include "func_registry.h"
#include <stdio.h>

// 演示如何使用优雅的函数注册系统

void demo_func_registry_usage(void)
{
    // 初始化函数注册表
    init_func_registry();
    
    printf("=== 优雅函数注册系统演示 ===\n");
    
    // 遍历并显示所有注册的函数
    int func_count = 0;
    for (int i = 0; i < 30 && func_list[i].func != NULL; i++) {
        printf("函数 %d: ID=%s, 输入数=%d, 输出数=%d\n", 
               i + 1, 
               func_list[i].id_func,
               func_list[i].input_num,
               func_list[i].output_num);
        func_count++;
    }
    
    printf("总共注册了 %d 个函数\n", func_count);
    printf("===========================\n");
}

int main()
{
    demo_func_registry_usage();
    return 0;
}
