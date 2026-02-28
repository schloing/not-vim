#ifndef NV_CONTEXT_H
#define NV_CONTEXT_H

struct nv_window_node;
struct nv_view;
struct nv_buff;

struct nv_context {
    struct nv_window_node* window;
    struct nv_view* view;
    struct nv_buff* buffer;
};

#endif