#include "func_public.h"
#include "func_registry.h"
#include "tree_utils.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Balanced-tree registry for mode functions                          */
/* ------------------------------------------------------------------ */

static tree_ctx_t func_tree;

/* Compare two tree_node_t entries by their key string */
static int func_compare(const void *a, const void *b)
{
    const tree_node_t *na = (const tree_node_t *)a;
    const tree_node_t *nb = (const tree_node_t *)b;
    return strcmp(na->key, nb->key);
}

/* Free a single tree_node_t (key string + node itself) */
static void func_free_node(void *node)
{
    tree_node_t *n = (tree_node_t *)node;
    free(n->key);
    free(n);
}

/* ---------- public API ---------- */

void register_module_functions(struct func_attribute (*(*funcs)(void)), char *func_id)
{
    if (funcs == NULL || func_id == NULL || func_id[0] == '\0') {
        return;
    }

    tree_node_t *new_node = malloc(sizeof(tree_node_t));
    if (new_node == NULL) return;

    new_node->key = strdup(func_id);
    if (new_node->key == NULL) {
        free(new_node);
        return;
    }
    new_node->func = funcs;

    void *existing = NULL;
    int rc = tree_insert(&func_tree, new_node, &existing);
    if (rc < 0) {
        /* tsearch allocation failure */
        free(new_node->key);
        free(new_node);
        return;
    }
    if (rc > 0) {
        /* key already exists — discard new node, keep existing */
        free(new_node->key);
        free(new_node);
    }
}

REG_MODE_FUNC_T find_function_by_id(const char *func_id)
{
    if (func_id == NULL || func_id[0] == '\0') return NULL;

    tree_node_t search_key = { .key = (char *)func_id, .func = NULL };
    tree_node_t *found = tree_find(&func_tree, &search_key);
    return found ? found->func : NULL;
}

int get_registered_func_count(void)
{
    return tree_count(&func_tree);
}

/* ---------- iteration ---------- */

static void (*func_walk_cb)(REG_MODE_FUNC_T func, void *arg) = NULL;
static void *func_walk_arg = NULL;

static void func_foreach_adapter(void *node, void *arg)
{
    (void)arg;
    tree_node_t *n = (tree_node_t *)node;
    if (func_walk_cb) func_walk_cb(n->func, func_walk_arg);
}

void iterate_all_functions(void (*callback)(REG_MODE_FUNC_T func, void *arg), void *arg)
{
    if (callback == NULL) return;
    func_walk_cb  = callback;
    func_walk_arg = arg;
    tree_foreach(&func_tree, func_foreach_adapter, NULL);
    func_walk_cb  = NULL;
    func_walk_arg = NULL;
}

/* ---------- cleanup ---------- */

void cleanup_func_registry(void)
{
    tree_cleanup(&func_tree, func_free_node);
}

/* ---------- height ---------- */

int get_tree_height_info(int *max_height)
{
    *max_height = tree_height(&func_tree);
    return *max_height;
}

/* ---------- initialisation ---------- */

__attribute__((constructor))
static void init_func_registry(void)
{
    tree_ctx_init(&func_tree, func_compare);
}
