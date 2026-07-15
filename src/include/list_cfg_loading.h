#ifndef LIST_CFG_LOADING_H
#define LIST_CFG_LOADING_H

#include "func_public.h"

/* Register/lookup a configuration node in the balanced tree */
int register_cfg_node(const char *cfg_id, struct my_node *node);
struct my_node *find_node_by_cfg_id(const char *cfg_id);
int get_registered_cfg_node_count(void);

/* Walk all registered cfg nodes */
void iterate_all_cfg_nodes(void (*callback)(struct my_node *node, void *arg), void *arg);

/* Free all cfg nodes and the tree itself (call at program exit) */
void cleanup_cfg_registry(void);

/* Get max height of the cfg tree (for diagnostics) */
int get_cfg_tree_height_info(int *max_height);

/* ------------------------------------------------------------------ */
/* Configuration-file parsing                                         */
/* ------------------------------------------------------------------ */

/*
 * Parse a flow-configuration file (JSON format).
 * `user_data` is forwarded to internal callbacks.
 * Returns 0 on success, -1 on error.
 */
int parse_flow_config(const char *filename, void *user_data);

/* Convenience entry point (demo / example usage) */
void parse_flow_config_example(void);

#endif
