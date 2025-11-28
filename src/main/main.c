#include "func_public.h"
#include "func_registry.h"
#include <stdio.h>

// 用于显示函数的回调函数
static void print_func_info(struct func_attribute *func, void *arg)
{
    static int count = 0;
    printf("函数 %d: ID=%s, 输入数=%d, 输出数=%d\n", 
           ++count, 
           func->id_func,
           func->input_num,
           func->output_num);
}

// 演示平衡树函数注册系统的使用
void demo_balanced_tree_registry(void)
{
    // 初始化函数注册表
    init_func_registry();
    
    printf("=== 平衡树函数注册系统演示 ===\n");
    printf("使用glibc tsearch/tfind实现零碰撞确定性结构\n\n");
    
    // 显示所有注册的函数
    printf("所有注册的函数:\n");
    iterate_all_functions(print_func_info, NULL);
    
    printf("总共注册了 %d 个函数\n", get_registered_func_count());
    
    // 演示平衡树查找功能
    printf("\n=== 平衡树查找演示 ===\n");
    
    // 查找存在的函数
    struct func_attribute *found_func = find_function_by_id("func_1");
    if (found_func != NULL) {
        printf("✅ 成功找到函数: ID=%s\n", found_func->id_func);
    } else {
        printf("❌ 未找到函数 func_1\n");
    }
    
    // 查找不存在的函数
    found_func = find_function_by_id("non_existent_func");
    if (found_func != NULL) {
        printf("❌ 找到函数: ID=%s (不应该发生)\n", found_func->id_func);
    } else {
        printf("✅ 未找到函数 non_existent_func (预期结果)\n");
    }
    
    // 演示平衡树性能特性
    printf("\n=== 平衡树性能特性 ===\n");
    int max_height = 0;
    get_tree_height_info(&max_height);
    printf("平衡树最大高度: %d\n", max_height);
}

int main()
{
    demo_balanced_tree_registry();
    
    // 程序退出前清理平衡树
    cleanup_func_registry();
    return 0;
}
