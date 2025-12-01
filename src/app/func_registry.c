#include "func_public.h"
#include "func_registry.h"
#include <stdlib.h>
#include <string.h>
#include <search.h>  // 包含tsearch/tfind等平衡树函数




// 全局平衡树根节点
static void *tree_root = NULL;
static int func_count = 0;

// 字符串比较函数 - 用于平衡树排序
static int string_compare(const void *a, const void *b)
{
    const tree_node_t *node_a = (const tree_node_t *)a;
    const tree_node_t *node_b = (const tree_node_t *)b;
    return strcmp(node_a->key, node_b->key);
}

// 模块注册函数
void register_module_functions(struct func_attribute (*(*funcs)(void)), char *func_id)
{
    if (funcs == NULL || func_id[0] == '\0') {
        return;
    }
    
    // 创建树节点
    tree_node_t *new_node = malloc(sizeof(tree_node_t));
    if (new_node == NULL) {
        return;
    }
    
    // 复制函数ID字符串
    new_node->key = strdup(func_id);
    if (new_node->key == NULL) {
        free(new_node);
        return;
    }
    
    new_node->func = funcs;
    
    // 插入到平衡树中
    void **result = tsearch(new_node, &tree_root, string_compare);
    if (result == NULL) {
        // 插入失败，清理内存
        free(new_node->key);
        free(new_node);
        return;
    }
    
    // 如果键已存在，tsearch会返回现有节点，我们需要释放新节点
    tree_node_t *existing_node = *(tree_node_t **)result;
    if (existing_node != new_node) {
        // 键已存在，释放新节点
        free(new_node->key);
        free(new_node);
        // 可以选择更新现有节点的函数指针，这里我们保持原样
    } else {
        // 新节点成功插入
        func_count++;
    }
}

// 根据函数ID查找函数
REG_MODE_FUNC_T find_function_by_id(const char *func_id)
{
    if (func_id == NULL || func_id[0] == '\0') {
        return NULL;
    }
    
    // 创建搜索键
    tree_node_t search_key = { .key = (char *)func_id, .func = NULL };
    
    // 在平衡树中查找
    void *result = tfind(&search_key, &tree_root, string_compare);
    if (result != NULL) {
        tree_node_t *found_node = *(tree_node_t **)result;
        return found_node->func;
    }
    
    return NULL;
}

// 获取注册的函数数量
int get_registered_func_count(void)
{
    return func_count;
}

// 遍历所有注册的函数（用于调试和显示）
static void (*current_callback)(REG_MODE_FUNC_T func, void *arg) = NULL;
static void *current_callback_arg = NULL;

static void walk_action(const void *node, VISIT order, int level) {
    if ((order == preorder || order == leaf) && current_callback != NULL) {
        const tree_node_t *tree_node = *(const tree_node_t **)node;
        current_callback(tree_node->func, current_callback_arg);
    }
}

void iterate_all_functions(void (*callback)(REG_MODE_FUNC_T func, void *arg), void *arg)
{
    if (callback == NULL) {
        return;
    }
    
    current_callback = callback;
    current_callback_arg = arg;
    twalk(tree_root, walk_action);
    current_callback = NULL;
    current_callback_arg = NULL;
}

// 清理平衡树（程序退出时调用）
static tree_node_t **node_array = NULL;
static size_t node_array_size = 0;
static size_t node_array_capacity = 0;

static void collect_node_action(const void *node, VISIT order, int level) {
    if (order == postorder || order == leaf) {
        tree_node_t *tree_node = *(tree_node_t **)node;
        if (node_array_size >= node_array_capacity) {
            size_t new_capacity = node_array_capacity == 0 ? 8 : node_array_capacity * 2;
            tree_node_t **new_array = realloc(node_array, new_capacity * sizeof(tree_node_t *));
            if (new_array == NULL) {
                return;
            }
            node_array = new_array;
            node_array_capacity = new_capacity;
        }
        node_array[node_array_size++] = tree_node;
    }
}

void cleanup_func_registry(void)
{
    // 收集所有节点指针
    node_array = NULL;
    node_array_size = 0;
    node_array_capacity = 0;
    twalk(tree_root, collect_node_action);
    
    // 对每个收集到的节点，从树中删除并释放内存
    for (size_t i = 0; i < node_array_size; i++) {
        tree_node_t *node = node_array[i];
        // 从树中删除节点（释放内部节点）
        tdelete(node, &tree_root, string_compare);
        // 释放节点内存
        free(node->key);
        free(node);
    }
    
    // 释放节点数组
    free(node_array);
    node_array = NULL;
    node_array_size = 0;
    node_array_capacity = 0;
    
    tree_root = NULL;
    func_count = 0;
}

// 平衡树性能测试专用函数 - 获取树的高度信息
static int *current_max_height = NULL;

static void height_action(const void *node, VISIT order, int level) {
    if (level > *current_max_height) {
        *current_max_height = level;
    }    
}

int get_tree_height_info(int *max_height)
{
    *max_height = 0;
    current_max_height = max_height;
    
    twalk(tree_root, height_action);
    current_max_height = NULL;
    return *max_height;
}
