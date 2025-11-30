#include "func_public.h"
#include "func_registry.h"
#include "list_cfg_loading.h"
#include <stdlib.h>
#include <string.h>
#include <search.h>  // 包含tsearch/tfind等平衡树函数
#include <yaml.h>    // yaml解析库

// 配置节点平衡树结构
typedef struct cfg_tree_node {
    char *key;                    // yaml配置中的id字符串
    struct my_node *node_ptr;     // 对应的my_node结构体地址
} cfg_tree_node_t;

// 全局配置平衡树根节点
static void *cfg_tree_root = NULL;
static int cfg_node_count = 0;

// 字符串比较函数 - 用于平衡树排序
static int cfg_string_compare(const void *a, const void *b)
{
    const cfg_tree_node_t *node_a = (const cfg_tree_node_t *)a;
    const cfg_tree_node_t *node_b = (const cfg_tree_node_t *)b;
    return strcmp(node_a->key, node_b->key);
}

// 注册配置节点到平衡树
void register_cfg_node(const char *cfg_id, struct my_node *node)
{
    if (cfg_id == NULL || cfg_id[0] == '\0' || node == NULL) {
        return;
    }
    
    // 创建树节点
    cfg_tree_node_t *new_node = malloc(sizeof(cfg_tree_node_t));
    if (new_node == NULL) {
        return;
    }
    
    // 复制配置ID字符串
    new_node->key = strdup(cfg_id);
    if (new_node->key == NULL) {
        free(new_node);
        return;
    }
    
    new_node->node_ptr = node;
    
    // 插入到平衡树中
    void **result = tsearch(new_node, &cfg_tree_root, cfg_string_compare);
    if (result == NULL) {
        // 插入失败，清理内存
        free(new_node->key);
        free(new_node);
        return;
    }
    
    // 如果键已存在，tsearch会返回现有节点，我们需要释放新节点
    cfg_tree_node_t *existing_node = *(cfg_tree_node_t **)result;
    if (existing_node != new_node) {
        // 键已存在，释放新节点
        free(new_node->key);
        free(new_node);
        // 可以选择更新现有节点的node指针，这里我们保持原样
    } else {
        // 新节点成功插入
        cfg_node_count++;
    }
}

// 根据配置ID查找节点
struct my_node *find_node_by_cfg_id(const char *cfg_id)
{
    if (cfg_id == NULL || cfg_id[0] == '\0') {
        return NULL;
    }
    
    // 创建搜索键
    cfg_tree_node_t search_key = { .key = (char *)cfg_id, .node_ptr = NULL };
    
    // 在平衡树中查找
    void *result = tfind(&search_key, &cfg_tree_root, cfg_string_compare);
    if (result != NULL) {
        cfg_tree_node_t *found_node = *(cfg_tree_node_t **)result;
        return found_node->node_ptr;
    }
    
    return NULL;
}

// 获取注册的配置节点数量
int get_registered_cfg_node_count(void)
{
    return cfg_node_count;
}

// 遍历所有注册的配置节点（用于调试和显示）
static void (*current_cfg_callback)(struct my_node *node, void *arg) = NULL;
static void *current_cfg_callback_arg = NULL;

static void cfg_walk_action(const void *node, VISIT order, int level) {
    if ((order == preorder || order == leaf) && current_cfg_callback != NULL) {
        const cfg_tree_node_t *tree_node = *(const cfg_tree_node_t **)node;
        current_cfg_callback(tree_node->node_ptr, current_cfg_callback_arg);
    }
}

void iterate_all_cfg_nodes(void (*callback)(struct my_node *node, void *arg), void *arg)
{
    if (callback == NULL) {
        return;
    }
    
    current_cfg_callback = callback;
    current_cfg_callback_arg = arg;
    twalk(cfg_tree_root, cfg_walk_action);
    current_cfg_callback = NULL;
    current_cfg_callback_arg = NULL;
}

// 清理配置平衡树（程序退出时调用）
static void free_cfg_node_action(const void *node, VISIT order, int level) {
    if (order == postorder || order == leaf) {
        cfg_tree_node_t *tree_node = *(cfg_tree_node_t **)node;
        free(tree_node->key);
        free(tree_node);
    }
}

void cleanup_cfg_registry(void)
{
    // 使用twalk遍历并释放所有节点
    twalk(cfg_tree_root, free_cfg_node_action);
    cfg_tree_root = NULL;
    cfg_node_count = 0;
}

// 配置平衡树性能测试专用函数 - 获取树的高度信息
static int *current_cfg_max_height = NULL;

static void cfg_height_action(const void *node, VISIT order, int level) {
    if (level > *current_cfg_max_height) {
        *current_cfg_max_height = level;
    }
}

int get_cfg_tree_height_info(int *max_height)
{
    *max_height = 0;
    current_cfg_max_height = max_height;
    
    twalk(cfg_tree_root, cfg_height_action);
    current_cfg_max_height = NULL;
    return *max_height;
}

// =============================================================================
// YAML 解析函数
// =============================================================================

// 解析 flow.yaml 文件的回调函数类型
typedef void (*yaml_block_callback_t)(const char *id, const char *type, const char *func, 
                                     void **list_array, int list_count,
                                     void **outputs_array, int outputs_count,
                                     void *user_data);

// 解析 flow.yaml 文件的主函数
int parse_flow_yaml(const char *filename, yaml_block_callback_t callback, void *user_data)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return -1;
    }

    yaml_parser_t parser;
    yaml_event_t event;
    int ret = 0;

    // 初始化解析器
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize YAML parser\n");
        fclose(file);
        return -1;
    }

    yaml_parser_set_input_file(&parser, file);

    // 解析状态变量
    int in_blocks = 0;
    int in_block = 0;
    int in_key = 0;
    char current_key[64] = {0};
    char current_id[64] = {0};
    char current_type[64] = {0};
    char current_func[64] = {0};
    
    void **current_list = NULL;
    int list_count = 0;
    void **current_outputs = NULL;
    int outputs_count = 0;

    // 开始解析
    do {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "YAML parser error\n");
            ret = -1;
            break;
        }

        switch (event.type) {
            case YAML_STREAM_START_EVENT:
                break;

            case YAML_DOCUMENT_START_EVENT:
                break;

            case YAML_MAPPING_START_EVENT:
                if (in_blocks && in_block) {
                    // 进入 block 内部的映射
                } else if (in_blocks) {
                    // 进入 block 项
                    in_block = 1;
                    memset(current_id, 0, sizeof(current_id));
                    memset(current_type, 0, sizeof(current_type));
                    memset(current_func, 0, sizeof(current_func));
                    list_count = 0;
                    outputs_count = 0;
                } else {
                    // 检查是否进入 blocks 映射
                    in_blocks = 1;
                }
                break;

            case YAML_MAPPING_END_EVENT:
                if (in_block) {
                    
                    in_block = 0;
                } else if (in_blocks) {
                    in_blocks = 0;
                }
                break;

            case YAML_SEQUENCE_START_EVENT:
                if (in_block && strcmp(current_key, "list") == 0) {
                    // 开始解析 list 数组
                } else if (in_block && strcmp(current_key, "outputs") == 0) {
                    // 开始解析 outputs 数组
                }
                break;

            case YAML_SEQUENCE_END_EVENT:
                break;

            case YAML_SCALAR_EVENT:
                if (in_block) {
                    if (!in_key) {
                        // 这是键
                        strncpy(current_key, (char*)event.data.scalar.value, 
                               sizeof(current_key) - 1);
                        in_key = 1;
                    } else {
                        // 这是值
                        if (strcmp(current_key, "id") == 0) {
                            strncpy(current_id, (char*)event.data.scalar.value, 
                                   sizeof(current_id) - 1);
                        } else if (strcmp(current_key, "type") == 0) {
                            strncpy(current_type, (char*)event.data.scalar.value, 
                                   sizeof(current_type) - 1);
                        } else if (strcmp(current_key, "func") == 0) {
                            strncpy(current_func, (char*)event.data.scalar.value, 
                                   sizeof(current_func) - 1);
                        }
                        in_key = 0;
                    }
                } else if (in_blocks && strcmp((char*)event.data.scalar.value, "blocks") == 0) {
                    // 找到 blocks 键
                }
                break;

            default:
                break;
        }

        yaml_event_delete(&event);

    } while (event.type != YAML_STREAM_END_EVENT);
    if (callback && current_id[0] != '\0') {
        callback(current_id, current_type, current_func, 
                current_list, list_count,
                current_outputs, outputs_count,
                user_data);
    }
    // 清理
    yaml_parser_delete(&parser);
    fclose(file);
    return ret;
}

// 示例回调函数 - 用于演示如何访问解析的数据
void example_block_callback(const char *id, const char *type, const char *func, 
                           void **list_array, int list_count,
                           void **outputs_array, int outputs_count,
                           void *user_data)
{
    printf("Block found:\n");
    printf("  ID: %s\n", id);
    printf("  Type: %s\n", type);
    if (func && func[0] != '\0') {
        printf("  Function: %s\n", func);
    }
    
    if (list_count > 0) {
        printf("  List items: %d\n", list_count);
        // 这里可以进一步解析 list 数组的内容
    }
    
    if (outputs_count > 0) {
        printf("  Outputs: %d\n", outputs_count);
        // 这里可以进一步解析 outputs 数组的内容
    }
    
    printf("\n");
}

// 预设调用变量的示例函数
void parse_flow_yaml_example(void)
{
    printf("=== 开始解析 flow.yaml ===\n");
    
    // 预设调用变量
    const char *filename = "src/flow.yaml";
    yaml_block_callback_t callback = example_block_callback;
    void *user_data = NULL;  // 可以传递任意用户数据
    
    // 调用解析函数
    int result = parse_flow_yaml(filename, callback, user_data);
    
    if (result == 0) {
        printf("=== 解析完成 ===\n");
    } else {
        printf("=== 解析失败 ===\n");
    }
}
