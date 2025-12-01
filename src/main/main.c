#include "func_public.h"
#include "func_registry.h"
#include <stdio.h>
#include "list_cfg_loading.h"

// 用于显示函数的回调函数
static void print_func_info(REG_MODE_FUNC_T func, void *arg)
{
    static int count = 0;
    struct func_attribute *self = func();
    printf("函数 %d: 输入数=%d, 输出数=%d\n", 
           ++count, 
           self->input_num,
           self->output_num);
    
    // 释放分配的内存
    if (self != NULL) {
        if (self->arg != NULL) {
            free(self->arg);
        }
        free(self);
    }
}

// 用于显示配置节点的回调函数
static void print_cfg_node_info(struct my_node *node, void *arg)
{
    static int count = 0;
    printf("配置节点 %d: ID=%s, 类型=%d\n", 
           ++count, 
           node->id_name,
           node->type);
}

// 演示平衡树函数注册系统的使用
void demo_balanced_tree_registry(void)
{
    
    printf("=== 平衡树函数注册系统演示 ===\n");
    printf("使用glibc tsearch/tfind实现零碰撞确定性结构\n\n");
    
    // 显示所有注册的函数
    printf("所有注册的函数:\n");
    iterate_all_functions(print_func_info, NULL);
    
    printf("总共注册了 %d 个函数\n", get_registered_func_count());
    
    // 演示平衡树查找功能
    printf("\n=== 平衡树查找演示 ===\n");
    
    // 查找存在的函数
    REG_MODE_FUNC_T found_func = find_function_by_id("mode_1");
    if (found_func != NULL) {
        printf("✅ 成功找到函数: ID=%s\n", "mode_1");
    } else {
        printf("❌ 未找到函数 mode_1\n");
    }
    
    // 查找不存在的函数
    found_func = find_function_by_id("non_existent_func");
    if (found_func != NULL) {
        printf("❌ 找到函数: ID=%s (不应该发生)\n", "non_existent_func");
    } else {
        printf("✅ 未找到函数 non_existent_func (预期结果)\n");
    }
    
    // 演示平衡树性能特性
    printf("\n=== 平衡树性能特性 ===\n");
    int max_height = 0;
    get_tree_height_info(&max_height);
    printf("平衡树最大高度: %d\n", max_height);
}

// 演示配置节点注册系统的使用
void demo_cfg_node_registry(void)
{
    printf("\n=== 配置节点平衡树系统演示 ===\n");
    
    // 显示所有注册的配置节点
    printf("所有注册的配置节点:\n");
    iterate_all_cfg_nodes(print_cfg_node_info, NULL);
    
    printf("总共注册了 %d 个配置节点\n", get_registered_cfg_node_count());
    
    // 演示配置节点查找功能
    printf("\n=== 配置节点查找演示 ===\n");
    
    // 查找存在的配置节点
    struct my_node *found_node = find_node_by_cfg_id("start1");
    if (found_node != NULL) {
        printf("✅ 成功找到配置节点: ID=%s, 类型=%d\n", found_node->id_name, found_node->type);
    } else {
        printf("❌ 未找到配置节点 start1\n");
    }
    
    // 查找不存在的配置节点
    found_node = find_node_by_cfg_id("non_existent_node");
    if (found_node != NULL) {
        printf("❌ 找到配置节点: ID=%s (不应该发生)\n", found_node->id_name);
    } else {
        printf("✅ 未找到配置节点 non_existent_node (预期结果)\n");
    }
    
    // 演示配置平衡树性能特性
    printf("\n=== 配置平衡树性能特性 ===\n");
    int cfg_max_height = 0;
    get_cfg_tree_height_info(&cfg_max_height);
    printf("配置平衡树最大高度: %d\n", cfg_max_height);
}

int main()
{
    demo_cfg_node_registry();
    demo_balanced_tree_registry();
    parse_flow_yaml_example();
    
    // 程序退出前清理平衡树
    cleanup_func_registry();
    cleanup_cfg_registry();
    return 0;
}
