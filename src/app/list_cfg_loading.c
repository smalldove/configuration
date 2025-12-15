#include "func_public.h"
#include "func_registry.h"
#include "list.h"
#include "list_cfg_loading.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // 用于strcasecmp
#include <search.h>  // 包含tsearch/tfind等平衡树函数
#include <yaml.h>    // yaml解析库
#include <ctype.h>   // 用于字符处理
#include <stdio.h>   // 用于文件操作
#include <cjson/cJSON.h> // cJSON库用于JSON解析

// 配置节点平衡树结构
typedef struct cfg_tree_node {
    char *key;                    // yaml配置中的id字符串
    struct my_node *node_ptr;     // 对应的my_node结构体地址
} cfg_tree_node_t;


struct _temp_out_info_list
{
    int arg_out;
    char *out_list;
    int arg_in;
    int num;
};


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
int register_cfg_node(const char *cfg_id, struct my_node *node)
{
    if (cfg_id == NULL || cfg_id[0] == '\0' || node == NULL) {
        return -1;
    }
    
    // 创建树节点
    cfg_tree_node_t *new_node = malloc(sizeof(cfg_tree_node_t));
    if (new_node == NULL) {
        return -2;
    }
    
    // 复制配置ID字符串
    new_node->key = strdup(cfg_id);
    if (new_node->key == NULL) {
        free(new_node);
        return -3;
    }
    
    new_node->node_ptr = node;
    
    // 插入到平衡树中
    void **result = tsearch(new_node, &cfg_tree_root, cfg_string_compare);
    if (result == NULL) {
        // 插入失败，清理内存
        free(new_node->key);
        free(new_node);
        return -4;
    }
    
    // 如果键已存在，tsearch会返回现有节点，我们需要释放新节点
    cfg_tree_node_t *existing_node = *(cfg_tree_node_t **)result;
    if (existing_node != new_node) {
        // 
        printf("%s 键已存在", new_node->key);
        free(new_node->key);
        free(new_node);
        return -5;
        // 可以选择更新现有节点的node指针，这里我们保持原样
    } else {
        // 新节点成功插入
        cfg_node_count++;
    }
    return 0;
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
static cfg_tree_node_t **cfg_node_array = NULL;
static size_t cfg_node_array_size = 0;
static size_t cfg_node_array_capacity = 0;

static void collect_cfg_node_action(const void *node, VISIT order, int level) {
    if (order == postorder || order == leaf) {
        cfg_tree_node_t *tree_node = *(cfg_tree_node_t **)node;
        if (cfg_node_array_size >= cfg_node_array_capacity) {
            size_t new_capacity = cfg_node_array_capacity == 0 ? 8 : cfg_node_array_capacity * 2;
            cfg_tree_node_t **new_array = realloc(cfg_node_array, new_capacity * sizeof(cfg_tree_node_t *));
            if (new_array == NULL) {
                return;
            }
            cfg_node_array = new_array;
            cfg_node_array_capacity = new_capacity;
        }
        cfg_node_array[cfg_node_array_size++] = tree_node;
    }
}

void cleanup_cfg_registry(void)
{
    // 收集所有节点指针
    cfg_node_array = NULL;
    cfg_node_array_size = 0;
    cfg_node_array_capacity = 0;
    twalk(cfg_tree_root, collect_cfg_node_action);
    
    // 对每个收集到的节点，从树中删除并释放内存
    for (size_t i = 0; i < cfg_node_array_size; i++) {
        cfg_tree_node_t *node = cfg_node_array[i];
        // 从树中删除节点（释放内部节点）
        tdelete(node, &cfg_tree_root, cfg_string_compare);
        
        // 释放 my_node 结构体
        if (node->node_ptr != NULL) {
            if (node->node_ptr->self) {
                struct func_attribute *self = node->node_ptr->self;
                if (self->arg != NULL) {
                    free(self->arg);
                }
                free(self);
            }
            
            if(node->node_ptr->out_list_table)
                free(node->node_ptr->out_list_table);

            if(node->node_ptr->out_list)
                free(node->node_ptr->out_list);

            free(node->node_ptr);
        }
        
        // 释放节点内存
        free(node->key);
        free(node);
    }
    
    // 释放节点数组
    free(cfg_node_array);
    cfg_node_array = NULL;
    cfg_node_array_size = 0;
    cfg_node_array_capacity = 0;
    
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
// 配置文件解析函数 (支持YAML和JSON)
// =============================================================================

// 解析配置文件的回调函数类型
typedef void (*config_block_callback_t)(const char *id, const char *type, const char *func, 
                                       void **list_array, int list_count,
                                       void **outputs_array, int outputs_count,
                                       void *user_data, struct my_node *node);

// 函数声明
static const char *get_file_extension(const char *filename);
static int parse_yaml_file(const char *filename, void *user_data);
static int parse_json_file(const char *filename, void *user_data);

// 获取文件扩展名
static const char *get_file_extension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return "";
    }
    return dot + 1;
}

// 解析 flow.yaml 文件的主函数
int parse_flow_yaml(const char *filename, void *user_data)
{
    // 检查文件扩展名
    // const char *ext = get_file_extension(filename);
    
    // if (strcasecmp(ext, "yaml") == 0 || strcasecmp(ext, "yml") == 0) {
    //     return parse_yaml_file(filename, callback, user_data);
    // } else if (strcasecmp(ext, "json") == 0) {
    //     return parse_json_file(filename, callback, user_data);
    // } else {
        // // 暂时先用json实现，yaml解析需优化
        // int result = parse_yaml_file(filename, callback, user_data);
        // if (result != 0) {
            int result = parse_json_file(filename, user_data);
        // }
        return result;
    // }
}

// 示例回调函数 - 用于演示如何访问解析的数据
int example_block_callback(const char *func, 
                           char **list_array, int list_count,
                           struct _temp_out_info_list *outputs_array, int outputs_count,
                           void *user_data, struct my_node *self)
{
    printf("Block found:\n");
    printf("  ID: %s\n", self->id_name);
    printf("  Type: %d\n", self->type);
    if (func && func[0] != '\0') {
        printf("  Function: %s\n", func);
    }

        // 这里可以根据 func 参数设置具体的函数指针，目前先初始化为默认值
    REG_MODE_FUNC_T reg_node_func = find_function_by_id(func);
    if(reg_node_func)
        self->self = reg_node_func();
    else
        return -1;

    /*串行链路链接*/
    for(int i = 0; i < list_count && i < LIST_MAX; i++)
    {
        self->next[i] = find_node_by_cfg_id(list_array[i]); 
        self->next_table[i] = find_node_by_cfg_id(list_array[i]); 
    }


    /*输出链路链接*/
    self->out_list = calloc(sizeof(struct out_list_t), 1);
    self->out_list_table = calloc(sizeof(struct out_list_t), 1);

    for(int i = 0; i < outputs_count && i < LIST_MAX; i++)
    {        
        self->out_list->node[i] = find_node_by_cfg_id(outputs_array[i].out_list);
        self->out_list->arg_in[i] = outputs_array[i].arg_in;
        self->out_list->arg_out[i] = outputs_array[i].arg_out;

        self->out_list_table->node[i] = find_node_by_cfg_id(outputs_array[i].out_list);
        self->out_list_table->arg_in[i] = outputs_array[i].arg_in;
        self->out_list_table->arg_out[i] = outputs_array[i].arg_out;

    }
    
    self->out_list->num = outputs_count >= LIST_MAX ? LIST_MAX : outputs_count;
    self->out_list_table->num = outputs_count >= LIST_MAX ? LIST_MAX : outputs_count;


    
    printf("  Node registered to global tree with ID: %s\n", self->id_name);
    printf("\n");
    return 0;
}

// 预设调用变量的示例函数
void parse_flow_yaml_example(void)
{
    printf("=== 开始解析链路配置 ===\n");
    const char *filename = "/home/xcvbnm/configuration/src/flow.json";
    void *user_data = cfg_tree_root;  // 可以传递任意用户数据
    
    // 调用解析函数
    int result = parse_flow_yaml(filename, user_data);
    
    if (result == 0) {
        printf("=== 解析完成 ===\n");
    } else {
        printf("=== 解析失败 ===\n");
    }
}

// =============================================================================
// JSON 解析函数实现 (使用cJSON库)
// =============================================================================

enum Node_type get_type(const char *type)
{
    enum Node_type ret = NODE_NON;
    if(strcmp(type, "start") == 0)
    {
        ret = NODE_START;
    }
    else if(strcmp(type, "relay") == 0)
    {
        ret = NODE_RELAY;
    }
    else if(strcmp(type, "branch") == 0)
    {
        ret = NODE_SWITCH;
    }
    else if(strcmp(type, "fanshaped") == 0)
    {
        ret = NODE_FANSHAPED;
    }
    else if(strcmp(type, "end") == 0)
    {
        ret = NODE_END;
    }
    
    
    return ret;
}

// 使用cJSON库解析JSON文件的函数
static int parse_json_file(const char *filename, void *user_data)
{
    if (filename == NULL) {
        return -1;
    }
    
    // 读取JSON文件内容
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open JSON file '%s'\n", filename);
        return -1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(file);
        fprintf(stderr, "Error: JSON file '%s' is empty\n", filename);
        return -1;
    }
    
    char *json_content = malloc(file_size + 1);
    if (json_content == NULL) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed for JSON file\n");
        return -1;
    }
    
    size_t bytes_read = fread(json_content, 1, file_size, file);
    json_content[bytes_read] = '\0';
    fclose(file);
    
    // 使用cJSON解析JSON
    cJSON *root = cJSON_Parse(json_content);
    free(json_content);
    
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
        }
        return -1;
    }
    
    // 查找blocks数组
    cJSON *blocks = cJSON_GetObjectItemCaseSensitive(root, "blocks");
    if (!cJSON_IsArray(blocks)) {
        fprintf(stderr, "Error: 'blocks' is not an array or not found\n");
        cJSON_Delete(root);
        return -1;
    }
    
    int block_count = 0;
    cJSON *block_item = NULL;
    
      // 遍历所有blocks

      /*先创建实例并注册到树中无其他操作*/
    cJSON_ArrayForEach(block_item, blocks) {
        if (!cJSON_IsObject(block_item)) {
            fprintf(stderr, "Warning: Block item is not an object, skipping\n");
            continue;
        }
        
        // 解析block中的字段
        cJSON *id_item = cJSON_GetObjectItemCaseSensitive(block_item, "id");
        cJSON *type_item = cJSON_GetObjectItemCaseSensitive(block_item, "type");

        if(NULL == id_item && NULL == type_item )
        {
            continue;
        }

        struct my_node *node = calloc(sizeof(struct my_node), 1);
        
        strncpy(node->id_name, id_item->valuestring, NODE_ID);
        node->type = get_type(type_item->valuestring);
        INIT_LIST_HEAD(&node->link);

        register_cfg_node(id_item->valuestring, node);

    }
    
    // 链接实例 保护链路与数据链路
    cJSON_ArrayForEach(block_item, blocks) {
        if (!cJSON_IsObject(block_item)) {
            fprintf(stderr, "Warning: Block item is not an object, skipping\n");
            continue;
        }
        
        // 解析block中的字段
        cJSON *id_item = cJSON_GetObjectItemCaseSensitive(block_item, "id");
        cJSON *type_item = cJSON_GetObjectItemCaseSensitive(block_item, "type");
        cJSON *func_item = cJSON_GetObjectItemCaseSensitive(block_item, "func");
        cJSON *list_item = cJSON_GetObjectItemCaseSensitive(block_item, "list");
        cJSON *outputs_item = cJSON_GetObjectItemCaseSensitive(block_item, "outputs");
        
        // 检查必需字段
        if (id_item == NULL || type_item == NULL || 
            !cJSON_IsString(id_item) || !cJSON_IsString(type_item)) {
            fprintf(stderr, "Warning: Block missing required 'id' or 'type' field, skipping\n");
            continue;
        }
        
        const char *id = id_item->valuestring;
        const char *func = (func_item != NULL && cJSON_IsString(func_item)) ? func_item->valuestring : "";
        struct my_node *tmp = find_node_by_cfg_id(id);
        if(NULL == tmp)
        {
            fprintf(stderr, "err no find the node : %s", id);
            continue;
        }
        // 解析list数组
        char **list_array = NULL;
        int list_count = 0, lists_count = 0;
        
        if (list_item != NULL && cJSON_IsArray(list_item)) {
            list_count = cJSON_GetArraySize(list_item);
            if (list_count > 0) {
                list_array = calloc(sizeof(char *) , (size_t)list_count);
                if (list_array != NULL) {
                    int i = 0;
                    cJSON *list_elem = NULL;
                    cJSON_ArrayForEach(list_elem, list_item) {
                        if (i >= list_count) break;
                        
                        if (cJSON_IsString(list_elem)) {
                            char *item_str = strdup(list_elem->valuestring);
                            list_array[i] = item_str;
                        } else {
                            printf("list 配置错误");
                        }
                        i++;
                    }
                }
            }
        }
        
        // 解析outputs数组
        struct _temp_out_info_list *outputs_array = NULL;
        int outputs_count = 0;
       
        
        if (outputs_item != NULL && cJSON_IsArray(outputs_item)) {
            outputs_count = cJSON_GetArraySize(outputs_item);
            if (outputs_count > 0) {
                outputs_array = calloc(sizeof(struct _temp_out_info_list) , (size_t)outputs_count);
                if (outputs_array != NULL) {
                    cJSON *output_elem = NULL;
                    cJSON *output_elems = NULL;
                    int i = 0, j = 0;

                    cJSON_ArrayForEach(output_elem, outputs_item) {

                        if (i >= outputs_count) break;

                        j = 0;
                        lists_count = cJSON_GetArraySize(output_elem);
                        if(lists_count == 3)
                        {
                            outputs_array[i].num = lists_count;
                            int k = 0;
                            cJSON_ArrayForEach(output_elems, output_elem) 
                            {
                                switch (k++) {
                                    case 0:
                                        outputs_array[i].arg_out = output_elems->valueint;

                                    break;

                                    case 1:
                                        outputs_array[i].out_list = output_elems->valuestring;

                                    break;

                                    case 2:
                                        outputs_array[i].arg_in = output_elems->valueint;
                                    break;
                                
                                }
                            }
                            j++;

                        }
                        else 
                        {
                            printf("配置错误");
                        }
                        
                        i++;
                    }
                }
            }
        }
        
        // 调用回调函数
        if(example_block_callback( func, list_array, list_count, outputs_array, outputs_count, user_data, tmp))
        {
            fprintf(stderr, "nofind:%s\n", func);
            // 从树中删除tmp对应的节点
            cfg_tree_node_t search_key;
            search_key.key = tmp->id_name;
            search_key.node_ptr = NULL;
            
            // 查找对应的树节点
            void *found = tfind(&search_key, &cfg_tree_root, cfg_string_compare);
            if (found != NULL) {
                cfg_tree_node_t *tree_node = *(cfg_tree_node_t **)found;
                
                // 从树中删除节点
                tdelete(&search_key, &cfg_tree_root, cfg_string_compare);
                
                // 释放树节点内存
                free(tree_node->key);
                free(tree_node);
                cfg_node_count--;
            }
            
            // 释放tmp节点内存
            if (tmp->self != NULL) {
                if (tmp->self->arg != NULL) {
                    free(tmp->self->arg);
                }
                free(tmp->self);
            }
            
            // 释放输出链表节点（如果列表已初始化）
            // 检查列表是否可能已被初始化（next不为NULL或列表不为空）
            if (tmp->out_list) 
            {
                free(tmp->out_list);
            }
            
            if (tmp->out_list_table) 
            {
                free(tmp->out_list_table);
            }
            
            free(tmp);
        }
        block_count++;
        
        // 清理当前block分配的内存
        if (list_array != NULL) {
            for (int i = 0; i < list_count; i++) {
                if (list_array[i] != NULL) {
                    free(list_array[i]);
                }
            }
            free(list_array);
        }
        
        if (outputs_array != NULL) {
            free(outputs_array);
        }
    }

    
    // 清理cJSON对象
    cJSON_Delete(root);
    
    if (block_count == 0) {
        fprintf(stderr, "Warning: No valid blocks found in JSON file\n");
        return -1;
    }
    
    printf("JSON parsing completed: %d blocks processed\n", block_count);
    return 0;
}

// =============================================================================
// YAML 解析函数实现 (占位符)
// =============================================================================

static int parse_yaml_file(const char *filename, void *user_data)
{
    // TODO: 实现 YAML 解析
    fprintf(stderr, "YAML parsing not yet implemented for file: %s\n", filename);
    return -1;
}
