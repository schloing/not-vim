#ifndef NV_WINDOW_H
#define NV_WINDOW_H

#include "cvector.h"
#include <stddef.h>

struct nv_window_node;

enum nv_window_node_split {
    NV_SPLIT_HORIZONTAL,
    NV_SPLIT_VERTICAL,
};

enum nv_window_kind {
    NV_WM_SPLIT,
    NV_WM_VIEW,
};

struct nv_window_area {
    float x;
    float y;
    float w;
    float h;
};

#define POOL_MANAGED

struct nv_window_node {
    size_t id;
    enum nv_window_kind kind;
    union {
        struct {
            POOL_MANAGED struct nv_window_node* left;
            POOL_MANAGED struct nv_window_node* right;
            enum nv_window_node_split kind;
            float ratio;
        } split;

        struct {
            POOL_MANAGED struct nv_view* view;
        } leaf;
    };
};

struct nv_context {
    struct nv_window_node* window;
    struct nv_view* view;
    struct nv_buff* buffer;
};

struct nv_window_node* nv_window_node_init();
struct nv_window_node* nv_window_node_push_child(struct nv_window_node* root, struct nv_window_node* child);
void nv_window_remove(struct nv_window_node* window);
void nv_window_resize_split(struct nv_window_node* split_node, float new_ratio);
struct nv_context nv_window_get_context(struct nv_window_node* node);
void nv_free_windows(struct nv_window_node* root);
struct nv_context nv_get_context(struct nv_window_node* window);

#endif
