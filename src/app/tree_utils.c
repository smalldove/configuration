#include "tree_utils.h"
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Single file-scope context for twalk callbacks.                      */
/* twalk() does not accept a user-data pointer, so this is the cleanest */
/* way to eliminate duplicated globals across every tree user.          */
/* ------------------------------------------------------------------ */

/*
 * Thread-local walk context.
 * twalk() does not accept a user-data pointer, so each thread maintains
 * its own context to allow concurrent operations on different trees.
 */
static _Thread_local struct {
    /* tree_foreach */
    void (*callback)(void *node, void *arg);
    void *cb_arg;

    /* tree_cleanup */
    void **node_array;
    size_t node_array_size;
    size_t node_array_capacity;

    /* tree_height */
    int max_height;
} wk;

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void tree_ctx_init(tree_ctx_t *ctx, int (*compare)(const void *, const void *))
{
    ctx->root    = NULL;
    ctx->count   = 0;
    ctx->compare = compare;
}

/* ---------- insert ---------- */

int tree_insert(tree_ctx_t *ctx, void *new_node, void **existing_out)
{
    void **result = tsearch(new_node, &ctx->root, ctx->compare);
    if (result == NULL) {
        if (existing_out) *existing_out = NULL;
        return -1;
    }

    void *found = *result;
    if (found != new_node) {
        /* key already exists */
        if (existing_out) *existing_out = found;
        return 1;
    }

    /* new node inserted */
    ctx->count++;
    if (existing_out) *existing_out = new_node;
    return 0;
}

/* ---------- find ---------- */

void *tree_find(tree_ctx_t *ctx, const void *search_key)
{
    void *result = tfind(search_key, &ctx->root, ctx->compare);
    if (result == NULL) return NULL;
    return *(void **)result;
}

/* ---------- foreach ---------- */

static void foreach_action(const void *node, VISIT order, int level)
{
    (void)level;
    if ((order == preorder || order == leaf) && wk.callback != NULL) {
        void *tree_node = *(void **)node;
        wk.callback(tree_node, wk.cb_arg);
    }
}

void tree_foreach(tree_ctx_t *ctx, void (*callback)(void *node, void *arg), void *arg)
{
    if (callback == NULL) return;
    wk.callback = callback;
    wk.cb_arg   = arg;
    twalk(ctx->root, foreach_action);
    wk.callback = NULL;
    wk.cb_arg   = NULL;
}

/* ---------- cleanup ---------- */

static void collect_action(const void *node, VISIT order, int level)
{
    (void)level;
    if (order == postorder || order == leaf) {
        void *tree_node = *(void **)node;
        if (wk.node_array_size >= wk.node_array_capacity) {
            size_t new_cap = wk.node_array_capacity == 0 ? 8 : wk.node_array_capacity * 2;
            void **new_arr = realloc(wk.node_array, new_cap * sizeof(void *));
            if (new_arr == NULL) return;
            wk.node_array      = new_arr;
            wk.node_array_capacity = new_cap;
        }
        wk.node_array[wk.node_array_size++] = tree_node;
    }
}

void tree_cleanup(tree_ctx_t *ctx, void (*free_node)(void *node))
{
    /* Collect all node pointers via twalk */
    wk.node_array     = NULL;
    wk.node_array_size = 0;
    wk.node_array_capacity = 0;
    twalk(ctx->root, collect_action);

    /* Remove each node from the tree and free it */
    for (size_t i = 0; i < wk.node_array_size; i++) {
        void *node = wk.node_array[i];
        tdelete(node, &ctx->root, ctx->compare);
        if (free_node) free_node(node);
    }

    free(wk.node_array);
    wk.node_array     = NULL;
    wk.node_array_size = 0;
    wk.node_array_capacity = 0;

    ctx->root  = NULL;
    ctx->count = 0;
}

/* ---------- count ---------- */

int tree_count(const tree_ctx_t *ctx)
{
    return ctx->count;
}

/* ---------- height ---------- */

static void height_action(const void *node, VISIT order, int level)
{
    (void)node;
    (void)order;
    if (level > wk.max_height) {
        wk.max_height = level;
    }
}

int tree_height(tree_ctx_t *ctx)
{
    wk.max_height = 0;
    twalk(ctx->root, height_action);
    return wk.max_height;
}
