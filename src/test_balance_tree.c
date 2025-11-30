#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_cfg_loading.h"

// 测试回调函数 - 用于遍历显示所有节点
void test_callback(struct my_node *node, void *arg)
{
    if (node != NULL) {
        printf("  Node ID: %s, Type: %d\n", node->id_name, node->type);
    }
}

int main(void)
{
    printf("=== 平衡树机制测试 ===\n\n");
    
    // 创建一些测试节点
    struct my_node nodes[5];
    const char *cfg_ids[] = {"start1", "relay1", "relay2", "relay3", "relay4"};
    
    // 初始化测试节点
    for (int i = 0; i < 5; i++) {
        snprintf(nodes[i].id_name, sizeof(nodes[i].id_name), "node_%d", i + 1);
        nodes[i].type = i;  // 使用不同的类型进行测试
    }
    
    // 测试1: 注册节点到平衡树
    printf("1. 注册节点到平衡树:\n");
    for (int i = 0; i < 5; i++) {
        register_cfg_node(cfg_ids[i], &nodes[i]);
        printf("  注册节点: %s -> %s\n", cfg_ids[i], nodes[i].id_name);
    }
    printf("  注册节点总数: %d\n\n", get_registered_cfg_node_count());
    
    // 测试2: 查找节点
    printf("2. 查找节点测试:\n");
    for (int i = 0; i < 5; i++) {
        struct my_node *found = find_node_by_cfg_id(cfg_ids[i]);
        if (found != NULL) {
            printf("  找到节点: %s -> %s\n", cfg_ids[i], found->id_name);
        } else {
            printf("  未找到节点: %s\n", cfg_ids[i]);
        }
    }
    
    // 测试查找不存在的节点
    struct my_node *not_found = find_node_by_cfg_id("nonexistent");
    if (not_found == NULL) {
        printf("  正确: 未找到不存在的节点 'nonexistent'\n");
    }
    printf("\n");
    
    // 测试3: 遍历所有节点
    printf("3. 遍历所有注册节点:\n");
    iterate_all_cfg_nodes(test_callback, NULL);
    printf("\n");
    
    // 测试4: 树高度信息
    printf("4. 平衡树性能信息:\n");
    int max_height = 0;
    get_cfg_tree_height_info(&max_height);
    printf("  树的最大高度: %d\n", max_height);
    printf("  节点总数: %d\n\n", get_registered_cfg_node_count());
    
    // 测试5: 重复注册相同的键
    printf("5. 重复注册测试:\n");
    register_cfg_node("start1", &nodes[0]);  // 重复注册相同的键
    printf("  重复注册后节点总数: %d (应该保持不变)\n\n", get_registered_cfg_node_count());
    
    // 测试6: 清理平衡树
    printf("6. 清理平衡树:\n");
    cleanup_cfg_registry();
    printf("  清理后节点总数: %d\n", get_registered_cfg_node_count());
    
    // 验证清理后查找失败
    struct my_node *after_cleanup = find_node_by_cfg_id("start1");
    if (after_cleanup == NULL) {
        printf("  正确: 清理后无法找到任何节点\n");
    }
    
    printf("\n=== 平衡树测试完成 ===\n");
    return 0;
}
