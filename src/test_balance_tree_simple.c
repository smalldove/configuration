#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_cfg_loading.h"

// 显示节点的回调函数
void display_node_callback(struct my_node *node, void *arg)
{
    if (node != NULL) {
        const char *type_str = "unknown";
        switch (node->type) {
            case NODE_START: type_str = "start"; break;
            case NODE_END: type_str = "end"; break;
            case NODE_RELAY: type_str = "relay"; break;
            case NODE_FANSHAPED: type_str = "fanshaped"; break;
            case NODE_SWITCH: type_str = "switch"; break;
        }
        printf("  节点: %s (类型: %s)\n", node->id_name, type_str);
    }
}

int main(void)
{
    printf("=== 平衡树机制验证测试 ===\n\n");
    
    // 创建测试节点
    struct my_node nodes[5];
    const char *cfg_ids[] = {"start1", "relay1", "relay2", "relay3", "relay4"};
    
    // 初始化测试节点
    for (int i = 0; i < 5; i++) {
        strncpy(nodes[i].id_name, cfg_ids[i], sizeof(nodes[i].id_name) - 1);
        nodes[i].id_name[sizeof(nodes[i].id_name) - 1] = '\0';
        
        // 设置不同的类型
        if (strcmp(cfg_ids[i], "start1") == 0) {
            nodes[i].type = NODE_START;
        } else if (strcmp(cfg_ids[i], "relay3") == 0) {
            nodes[i].type = NODE_END;
        } else {
            nodes[i].type = NODE_RELAY;
        }
    }
    
    // 测试1: 注册节点到平衡树
    printf("1. 注册节点到平衡树:\n");
    for (int i = 0; i < 5; i++) {
        register_cfg_node(cfg_ids[i], &nodes[i]);
        printf("  注册节点: %s\n", cfg_ids[i]);
    }
    printf("  注册节点总数: %d\n\n", get_registered_cfg_node_count());
    
    // 测试2: 验证平衡树中的节点
    printf("2. 平衡树中的节点:\n");
    iterate_all_cfg_nodes(display_node_callback, NULL);
    printf("\n");
    
    // 测试3: 查找特定节点
    printf("3. 查找特定节点:\n");
    const char *search_ids[] = {"start1", "relay1", "relay2", "nonexistent"};
    for (int i = 0; i < 4; i++) {
        struct my_node *found = find_node_by_cfg_id(search_ids[i]);
        if (found != NULL) {
            printf("  找到节点: %s -> %s (类型: %d)\n", search_ids[i], found->id_name, found->type);
        } else {
            printf("  未找到节点: %s\n", search_ids[i]);
        }
    }
    printf("\n");
    
    // 测试4: 平衡树性能信息
    printf("4. 平衡树性能信息:\n");
    int max_height = 0;
    get_cfg_tree_height_info(&max_height);
    printf("  树的最大高度: %d\n", max_height);
    printf("  节点总数: %d\n\n", get_registered_cfg_node_count());
    
    // 测试5: 清理平衡树
    printf("5. 清理平衡树:\n");
    cleanup_cfg_registry();
    printf("  清理后节点总数: %d\n", get_registered_cfg_node_count());
    
    printf("\n=== 平衡树机制验证完成 ===\n");
    return 0;
}
