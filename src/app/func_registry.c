#include "func_public.h"
#include "func_registry.h"
#include <stdlib.h>
#include <string.h>
#include <search.h>  // 包含tsearch/tfind等平衡树函数

// 平衡树字典结构 - 零碰撞确定性结构
typedef struct tree_node {
    char *key;                    // 函数ID字符串
    struct func_attribute *func;  // 函数属性指针
} tree_node_t;

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
void register_module_functions(struct func_attribute *funcs)
{
    if (funcs == NULL || funcs->id_func[0] == '\0') {
        return;
    }
    
    // 创建树节点
    tree_node_t *new_node = malloc(sizeof(tree_node_t));
    if (new_node == NULL) {
        return;
    }
    
    // 复制函数ID字符串
    new_node->key = strdup(funcs->id_func);
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
struct func_attribute *find_function_by_id(const char *func_id)
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

// 初始化函数注册表
void init_func_registry(void)
{
    // 哈希表已经初始化为NULL，无需额外操作
    // 这个函数保留用于向后兼容性
}

// 获取注册的函数数量
int get_registered_func_count(void)
{
    return func_count;
}

// 遍历所有注册的函数（用于调试和显示）
static void (*current_callback)(struct func_attribute *func, void *arg) = NULL;
static void *current_callback_arg = NULL;

static void walk_action(const void *node, VISIT order, int level) {
    if ((order == preorder || order == leaf) && current_callback != NULL) {
        const tree_node_t *tree_node = *(const tree_node_t **)node;
        current_callback(tree_node->func, current_callback_arg);
    }
}

void iterate_all_functions(void (*callback)(struct func_attribute *func, void *arg), void *arg)
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
static void free_node_action(const void *node, VISIT order, int level) {
    if (order == postorder || order == leaf) {
        tree_node_t *tree_node = *(tree_node_t **)node;
        free(tree_node->key);
        free(tree_node);
    }
}

void cleanup_func_registry(void)
{
    // 使用twalk遍历并释放所有节点
    twalk(tree_root, free_node_action);
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
