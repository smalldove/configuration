#ifndef LIST_CFG_LOADING_H
#define LIST_CFG_LOADING_H

#include "func_public.h"

// 注册配置节点到平衡树
int register_cfg_node(const char *cfg_id, struct my_node *node);

// 根据配置ID查找节点
struct my_node *find_node_by_cfg_id(const char *cfg_id);

// 获取注册的配置节点数量
int get_registered_cfg_node_count(void);

// 遍历所有注册的配置节点（用于调试和显示）
void iterate_all_cfg_nodes(void (*callback)(struct my_node *node, void *arg), void *arg);

// 清理配置平衡树（程序退出时调用）
void cleanup_cfg_registry(void);

// 配置平衡树性能测试专用函数 - 获取树的高度信息
int get_cfg_tree_height_info(int *max_height);

// =============================================================================
// 配置文件解析函数 (支持YAML和JSON)
// =============================================================================

// 解析配置文件的回调函数类型
typedef void (*config_block_callback_t)(const char *id, const char *type, const char *func, 
                                       void **list_array, int list_count,
                                       void **outputs_array, int outputs_count,
                                       void *user_data, struct my_node *node);

// 解析 flow.yaml 文件的主函数 (支持YAML和JSON)
int parse_flow_yaml(const char *filename, void *user_data);

// 预设调用变量的示例函数
void parse_flow_yaml_example(void);

#endif
