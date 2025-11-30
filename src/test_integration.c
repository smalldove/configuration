#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_cfg_loading.h"

// YAML 解析回调函数 - 将解析的配置注册到平衡树
void yaml_to_balance_tree_callback(const char *id, const char *type, const char *func, 
                                  void **list_array, int list_count,
                                  void **outputs_array, int outputs_count,
                                  void *user_data)
{
    printf("解析到配置块: ID=%s, Type=%s\n", id, type);
    
    // 创建 my_node 结构体并注册到平衡树
    struct my_node *node = malloc(sizeof(struct my_node));
    if (node != NULL) {
        // 初始化节点
        strncpy(node->id_name, id, sizeof(node->id_name) - 1);
        node->id_name[sizeof(node->id_name) - 1] = '\0';
        
        // 根据类型设置节点类型
        if (strcmp(type, "start") == 0) {
            node->type = NODE_START;
        } else if (strcmp(type, "end") == 0) {
            node->type = NODE_END;
        } else if (strcmp(type, "relay") == 0) {
            node->type = NODE_RELAY;
        } else {
            node->type = NODE_RELAY; // 默认类型
        }
        
        // 注册到平衡树
        register_cfg_node(id, node);
        printf("  已注册到平衡树: %s\n", id);
    }
}

// 查找并显示节点的回调函数
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
    printf("=== YAML 配置与平衡树集成测试 ===\n\n");
    
    // 测试1: 解析 YAML 文件并注册到平衡树
    printf("1. 解析 YAML 文件并注册到平衡树:\n");
    const char *filename = "src/flow.yaml";
    int result = parse_flow_yaml(filename, yaml_to_balance_tree_callback, NULL);
    
    if (result == 0) {
        printf("  YAML 解析成功\n");
    } else {
        printf("  YAML 解析失败\n");
        return -1;
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
            printf("  找到节点: %s -> %s\n", search_ids[i], found->id_name);
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
    
    printf("\n=== 集成测试完成 ===\n");
    return 0;
}
