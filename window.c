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

struct nv_window* nv_find_empty_window(struct nv_window* root) {
    if (!root) {
        root = (struct nv_window*)nv_window_init();
        root->buffer = NULL;
    }

    if (!root->buffer) {
        root->buffer = (struct nv_buff*)_nv_alloc_buffer();
        return root;
    }

    root->left = (struct nv_window*)nv_find_empty_window(root->left);
    if (root->left) return root->left;
    root->right = (struct nv_window*)nv_find_empty_window(root->right);
    if (root->right) return root->right;

    return NULL;
}

void nv_open_window(struct nv_editor* editor, struct nv_window window) {
    return;
}
