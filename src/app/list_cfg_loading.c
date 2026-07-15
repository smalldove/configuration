#include "func_public.h"
#include "func_registry.h"
#include "list.h"
#include "list_cfg_loading.h"
#include "tree_utils.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>
#include <cjson/cJSON.h>

/* ------------------------------------------------------------------ */
/* Configuration-node balanced tree                                    */
/* ------------------------------------------------------------------ */

typedef struct cfg_tree_node {
    char           *key;
    struct my_node *node_ptr;
} cfg_tree_node_t;

static tree_ctx_t cfg_tree;

static int cfg_compare(const void *a, const void *b)
{
    const cfg_tree_node_t *na = (const cfg_tree_node_t *)a;
    const cfg_tree_node_t *nb = (const cfg_tree_node_t *)b;
    return strcmp(na->key, nb->key);
}

/* ---------- public cfg-tree API ---------- */

int register_cfg_node(const char *cfg_id, struct my_node *node)
{
    if (cfg_id == NULL || cfg_id[0] == '\0' || node == NULL) return -1;

    cfg_tree_node_t *new_node = malloc(sizeof(cfg_tree_node_t));
    if (new_node == NULL) return -2;

    new_node->key = strdup(cfg_id);
    if (new_node->key == NULL) {
        free(new_node);
        return -3;
    }
    new_node->node_ptr = node;

    void *existing = NULL;
    int rc = tree_insert(&cfg_tree, new_node, &existing);
    if (rc < 0) {
        free(new_node->key);
        free(new_node);
        return -4;
    }
    if (rc > 0) {
        printf("Duplicate key: %s\n", new_node->key);
        free(new_node->key);
        free(new_node);
        return -5;
    }
    return 0;
}

struct my_node *find_node_by_cfg_id(const char *cfg_id)
{
    if (cfg_id == NULL || cfg_id[0] == '\0') return NULL;

    cfg_tree_node_t search_key = { .key = (char *)cfg_id, .node_ptr = NULL };
    cfg_tree_node_t *found = tree_find(&cfg_tree, &search_key);
    return found ? found->node_ptr : NULL;
}

int get_registered_cfg_node_count(void)
{
    return tree_count(&cfg_tree);
}

/* ---------- iteration ---------- */

static void (*cfg_walk_cb)(struct my_node *node, void *arg) = NULL;
static void *cfg_walk_arg = NULL;

static void cfg_foreach_adapter(void *node, void *arg)
{
    (void)arg;
    cfg_tree_node_t *n = (cfg_tree_node_t *)node;
    if (cfg_walk_cb) cfg_walk_cb(n->node_ptr, cfg_walk_arg);
}

void iterate_all_cfg_nodes(void (*callback)(struct my_node *node, void *arg), void *arg)
{
    if (callback == NULL) return;
    cfg_walk_cb  = callback;
    cfg_walk_arg = arg;
    tree_foreach(&cfg_tree, cfg_foreach_adapter, NULL);
    cfg_walk_cb  = NULL;
    cfg_walk_arg = NULL;
}

/* ---------- resource helpers (shared by cleanup and error paths) ---------- */

/* Free all resources owned by a my_node.  Does NOT remove from the cfg tree. */
static void free_my_node_resources(struct my_node *node)
{
    if (node == NULL) return;

    if (node->self) {
        free(node->self->arg);
        free(node->self);
    }
    free(node->out_list);
    free(node->out_list_table);
    free(node);
}

/* Free a cfg_tree_node (key + wrapper, NOT the my_node payload). */
static void free_cfg_tree_node(void *node)
{
    cfg_tree_node_t *n = (cfg_tree_node_t *)node;
    if (n) {
        free_my_node_resources(n->node_ptr);
        free(n->key);
        free(n);
    }
}

void cleanup_cfg_registry(void)
{
    tree_cleanup(&cfg_tree, free_cfg_tree_node);
}

/* ---------- height ---------- */

int get_cfg_tree_height_info(int *max_height)
{
    *max_height = tree_height(&cfg_tree);
    return *max_height;
}

/* ---------- initialisation ---------- */

__attribute__((constructor))
static void init_cfg_registry(void)
{
    tree_ctx_init(&cfg_tree, cfg_compare);
}

/* =============================================================================
 * Configuration-file parsing (JSON)
 * ============================================================================= */

struct _temp_out_info_list {
    int   arg_out;
    char *out_list;
    int   arg_in;
    int   num;
};

static int parse_json_file(const char *filename, void *user_data);

/* Parse a flow-configuration file. Currently JSON-only; YAML is a stub. */
int parse_flow_config(const char *filename, void *user_data)
{
    return parse_json_file(filename, user_data);
}

/* ---------- block callback (wires up parsed data to my_node) ---------- */

static int example_block_callback(const char *func,
                                  char **list_array, int list_count,
                                  struct _temp_out_info_list *outputs_array, int outputs_count,
                                  void *user_data, struct my_node *self)
{
    (void)user_data;

    printf("Block found:\n");
    printf("  ID: %s\n", self->id_name);
    printf("  Type: %d\n", self->type);
    if (func && func[0] != '\0') {
        printf("  Function: %s\n", func);
    }

    REG_MODE_FUNC_T reg_node_func = find_function_by_id(func);
    if (reg_node_func)
        self->self = reg_node_func();
    else
        return -1;

    /* Serial link wiring */
    for (int i = 0; i < list_count && i < LIST_MAX; i++) {
        self->next[i]       = find_node_by_cfg_id(list_array[i]);
        self->next_table[i] = find_node_by_cfg_id(list_array[i]);
    }

    /* Output link wiring */
    self->out_list       = calloc(sizeof(struct out_list_t), 1);
    self->out_list_table = calloc(sizeof(struct out_list_t), 1);
    if (self->out_list == NULL || self->out_list_table == NULL) {
        free(self->out_list);
        free(self->out_list_table);
        return -1;
    }

    for (int i = 0; i < outputs_count && i < LIST_MAX; i++) {
        self->out_list->node[i]       = find_node_by_cfg_id(outputs_array[i].out_list);
        self->out_list->arg_in[i]     = outputs_array[i].arg_in;
        self->out_list->arg_out[i]    = outputs_array[i].arg_out;

        self->out_list_table->node[i]    = find_node_by_cfg_id(outputs_array[i].out_list);
        self->out_list_table->arg_in[i]  = outputs_array[i].arg_in;
        self->out_list_table->arg_out[i] = outputs_array[i].arg_out;
    }

    self->out_list->num       = (outputs_count >= LIST_MAX) ? LIST_MAX : outputs_count;
    self->out_list_table->num = (outputs_count >= LIST_MAX) ? LIST_MAX : outputs_count;

    printf("  Node registered to global tree with ID: %s\n", self->id_name);
    printf("\n");
    return 0;
}

/* ---------- convenience entry point (demo) ---------- */

static const char *resolve_flow_json_path(void)
{
    static const char *candidates[] = {
        "../src/flow.json",              /* when run from project/ */
        "src/flow.json",                 /* when run from repo root */
        "./flow.json",
        NULL
    };

    for (int i = 0; candidates[i] != NULL; i++) {
        FILE *fp = fopen(candidates[i], "r");
        if (fp != NULL) {
            fclose(fp);
            return candidates[i];
        }
    }
    return candidates[0];
}

void parse_flow_config_example(void)
{
    printf("=== Start parsing flow configuration ===\n");
    const char *filename = resolve_flow_json_path();
    void *user_data = NULL;

    printf("Using flow config: %s\n", filename);
    int result = parse_flow_config(filename, user_data);
    if (result == 0) {
        printf("=== Parse completed ===\n");
    } else {
        printf("=== Parse failed ===\n");
    }
}

/* =============================================================================
 * JSON parsing (cJSON)
 * ============================================================================= */

enum Node_type get_type(const char *type)
{
    if (strcmp(type, "start") == 0)     return NODE_START;
    if (strcmp(type, "relay") == 0)     return NODE_RELAY;
    if (strcmp(type, "branch") == 0)    return NODE_SWITCH;
    if (strcmp(type, "fanshaped") == 0) return NODE_FANSHAPED;
    if (strcmp(type, "end") == 0)       return NODE_END;
    return NODE_NON;
}

static int parse_json_file(const char *filename, void *user_data)
{
    if (filename == NULL) return -1;

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

    size_t size = (size_t)file_size;
    char *json_content = malloc(size + 1);
    if (json_content == NULL) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed for JSON file\n");
        return -1;
    }

    size_t bytes_read = fread(json_content, 1, size, file);
    json_content[bytes_read] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(json_content);
    free(json_content);

    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
        }
        return -1;
    }

    cJSON *blocks = cJSON_GetObjectItemCaseSensitive(root, "blocks");
    if (!cJSON_IsArray(blocks)) {
        fprintf(stderr, "Error: 'blocks' is not an array or not found\n");
        cJSON_Delete(root);
        return -1;
    }

    int block_count = 0;
    cJSON *block_item = NULL;

    /* ---- Phase 1: create nodes and register in cfg tree ---- */
    cJSON_ArrayForEach(block_item, blocks) {
        if (!cJSON_IsObject(block_item)) {
            fprintf(stderr, "Warning: Block item is not an object, skipping\n");
            continue;
        }

        cJSON *id_item   = cJSON_GetObjectItemCaseSensitive(block_item, "id");
        cJSON *type_item = cJSON_GetObjectItemCaseSensitive(block_item, "type");

        if (id_item == NULL || type_item == NULL) continue;

        struct my_node *node = calloc(sizeof(struct my_node), 1);
        strncpy(node->id_name, id_item->valuestring, NODE_ID);
        node->type = get_type(type_item->valuestring);
        INIT_LIST_HEAD(&node->link);

        register_cfg_node(id_item->valuestring, node);
    }

    /* ---- Phase 2: wire up links and functions ---- */
    cJSON_ArrayForEach(block_item, blocks) {
        if (!cJSON_IsObject(block_item)) {
            fprintf(stderr, "Warning: Block item is not an object, skipping\n");
            continue;
        }

        cJSON *id_item      = cJSON_GetObjectItemCaseSensitive(block_item, "id");
        cJSON *type_item    = cJSON_GetObjectItemCaseSensitive(block_item, "type");
        cJSON *func_item    = cJSON_GetObjectItemCaseSensitive(block_item, "func");
        cJSON *list_item    = cJSON_GetObjectItemCaseSensitive(block_item, "list");
        cJSON *outputs_item = cJSON_GetObjectItemCaseSensitive(block_item, "outputs");

        if (id_item == NULL || type_item == NULL ||
            !cJSON_IsString(id_item) || !cJSON_IsString(type_item)) {
            fprintf(stderr, "Warning: Block missing required 'id' or 'type' field, skipping\n");
            continue;
        }

        const char *id   = id_item->valuestring;
        const char *func = (func_item != NULL && cJSON_IsString(func_item))
                            ? func_item->valuestring : "";

        struct my_node *tmp = find_node_by_cfg_id(id);
        if (tmp == NULL) {
            fprintf(stderr, "Error: node not found: %s\n", id);
            continue;
        }

        /* --- parse list array --- */
        char **list_array = NULL;
        int list_count = 0;

        if (list_item != NULL && cJSON_IsArray(list_item)) {
            list_count = cJSON_GetArraySize(list_item);
            if (list_count > 0) {
                list_array = calloc(sizeof(char *), (size_t)list_count);
                if (list_array != NULL) {
                    int i = 0;
                    cJSON *list_elem = NULL;
                    cJSON_ArrayForEach(list_elem, list_item) {
                        if (i >= list_count) break;
                        if (cJSON_IsString(list_elem)) {
                            list_array[i] = strdup(list_elem->valuestring);
                        } else {
                            fprintf(stderr, "Error: invalid list element in config\n");
                        }
                        i++;
                    }
                }
            }
        }

        /* --- parse outputs array --- */
        struct _temp_out_info_list *outputs_array = NULL;
        int outputs_count = 0;

        if (outputs_item != NULL && cJSON_IsArray(outputs_item)) {
            outputs_count = cJSON_GetArraySize(outputs_item);
            if (outputs_count > 0) {
                outputs_array = calloc(sizeof(struct _temp_out_info_list), (size_t)outputs_count);
                if (outputs_array != NULL) {
                    cJSON *output_elem  = NULL;
                    cJSON *output_elems = NULL;
                    int i = 0;

                    cJSON_ArrayForEach(output_elem, outputs_item) {
                        if (i >= outputs_count) break;

                        int elem_count = cJSON_GetArraySize(output_elem);
                        if (elem_count == 3) {
                            outputs_array[i].num = elem_count;
                            int k = 0;
                            cJSON_ArrayForEach(output_elems, output_elem) {
                                switch (k++) {
                                case 0: outputs_array[i].arg_out  = output_elems->valueint; break;
                                case 1: outputs_array[i].out_list = output_elems->valuestring; break;
                                case 2: outputs_array[i].arg_in   = output_elems->valueint; break;
                                }
                            }
                        } else {
                            fprintf(stderr, "Error: invalid outputs configuration\n");
                        }
                        i++;
                    }
                }
            }
        }

        /* --- wire up the node --- */
        if (example_block_callback(func, list_array, list_count,
                                   outputs_array, outputs_count, user_data, tmp)) {
            fprintf(stderr, "Function not found: %s\n", func);

            /* Remove node from tree and free its resources */
            cfg_tree_node_t search_key;
            search_key.key     = tmp->id_name;
            search_key.node_ptr = NULL;

            cfg_tree_node_t *tree_node = tree_find(&cfg_tree, &search_key);
            if (tree_node != NULL) {
                tdelete(&search_key, &cfg_tree.root, cfg_tree.compare);
                cfg_tree.count--;

                /* Only free the cfg wrapper — my_node is tmp itself */
                free(tree_node->key);
                free(tree_node);

                free_my_node_resources(tmp);
            }
        }
        block_count++;

        /* --- free temporary parse buffers --- */
        if (list_array != NULL) {
            for (int i = 0; i < list_count; i++) free(list_array[i]);
            free(list_array);
        }
        free(outputs_array);
    }

    cJSON_Delete(root);

    if (block_count == 0) {
        fprintf(stderr, "Warning: No valid blocks found in JSON file\n");
        return -1;
    }

    printf("JSON parsing completed: %d blocks processed\n", block_count);
    return 0;
}

/* YAML parsing is not implemented yet; parse_flow_config is JSON-only. */
