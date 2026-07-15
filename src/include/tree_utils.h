#ifndef TREE_UTILS_H
#define TREE_UTILS_H

#include <search.h>

/*
 * Generic balanced-tree context (wraps glibc tsearch/tfind/twalk/tdelete).
 *
 * Encapsulates a binary search tree with a single file-scope walk context,
 * eliminating the need for per-file global variables when using twalk.
 *
 * Usage:
 *   1. Define a node struct with a `char *key` field (or any comparable key).
 *   2. Provide a strcmp-based compare function.
 *   3. Use tree_insert / tree_find / tree_foreach / tree_cleanup.
 */

typedef struct tree_ctx {
    void *root;
    int   count;
    int (*compare)(const void *, const void *);
} tree_ctx_t;

/* Initialize a tree context. Must be called before any other operations. */
void tree_ctx_init(tree_ctx_t *ctx, int (*compare)(const void *, const void *));

/*
 * Insert `new_node` into the tree.
 * Returns:
 *   0 — inserted successfully (caller transfers ownership to the tree)
 *   1 — key already exists, *existing_out set to the existing node
 *  -1 — error (allocation failure in tsearch)
 */
int tree_insert(tree_ctx_t *ctx, void *new_node, void **existing_out);

/* Find a node by `search_key`. Returns the found node or NULL. */
void *tree_find(tree_ctx_t *ctx, const void *search_key);

/*
 * Walk all nodes. `callback` is invoked exactly once per node.
 * `arg` is passed through as the second parameter to callback.
 */
void tree_foreach(tree_ctx_t *ctx, void (*callback)(void *node, void *arg), void *arg);

/*
 * Remove all nodes from the tree and free each one via `free_node(node)`.
 * The free_node callback should free the node's key and the node itself.
 */
void tree_cleanup(tree_ctx_t *ctx, void (*free_node)(void *node));

/* Return the number of nodes currently in the tree. */
int tree_count(const tree_ctx_t *ctx);

/* Compute the maximum height of the tree. Writes result to *height and returns it. */
int tree_height(tree_ctx_t *ctx);

#endif /* TREE_UTILS_H */
