#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "editor.h"
#include "termbox2.h"
#include "window.h"

static struct nv_buff* _nv_alloc_buffer();

struct nv_window* nv_window_init() {
    struct nv_window* window = (struct nv_window*)calloc(1, sizeof(struct nv_window));
    window->buffer = NULL;
    window->left = NULL;
    window->parent = NULL;
    window->right = NULL;
    return window;
}

static struct nv_buff* _nv_alloc_buffer() {
    struct nv_buff* buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));
    nv_buffer_init(buffer, "");
    return buffer;
}

void nv_free_windows(struct nv_window* root) {
    if (!root) return;

    nv_free_buffer(root->buffer);
    nv_free_windows(root->left);
    nv_free_windows(root->right);
    
    free(root);
    root = NULL;
}

static struct nv_window* _nv_assign_child(struct nv_window* root, struct nv_window** child) {
    *child = (struct nv_window*)nv_find_empty_window(*child);

    if (*child) {
        (*child)->parent = root;
        return *child;
    }

    return NULL;
}

struct nv_window* nv_find_empty_window(struct nv_window* root) {
    if (!root) {
        root = (struct nv_window*)nv_window_init();
        root->buffer = NULL;
        root->parent = NULL;
    }

    if (!root->buffer) {
        root->buffer = (struct nv_buff*)_nv_alloc_buffer();
        return root;
    }

    return _nv_assign_child(root, &root->left) ? root->left : _nv_assign_child(root, &root->right);

    return NULL;
}

void nv_redistribute(struct nv_window* root) {
    if (!root) return;
    struct nv_window** forked = NULL;
    
    if (!root->left) forked = &root->left;
    if (!root->right) forked = &root->right;

    if (!forked) {
        *forked = root;
        (*forked)->wd.h = root->max_h;
        (*forked)->wd.w = (int)root->max_w / 2;
        (*forked)->w = 5;
        (*forked)->h = 10;
    }

    root = NULL;
}

void nv_open_window(struct nv_editor* editor, struct nv_window window) {
    return;
}
