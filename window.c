#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "editor.h"
#include "termbox2.h"
#include "window.h"

struct nv_window* nv_window_init() {
    struct nv_window* window = (struct nv_window*)calloc(1, sizeof(struct nv_window));
    window->buffer = NULL;
    window->left = NULL;
    window->parent = NULL;
    window->right = NULL;
    return window;
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
        root->active = true;
        return root;
    }

    if (!root->active) // root window is allocated but not active
        return root;

    return _nv_assign_child(root, &root->left) ? root->left : _nv_assign_child(root, &root->right);
}

void nv_redistribute(struct nv_window* root) {
    if (!root) return;
    struct nv_window** forked = NULL;
    struct nv_window* other;
    
    if (!root->left) {
        forked = &root->left;
        other = root->right;
    }

    if (!root->right) {
        forked = &root->right;
        other = root->left;
    }

    if (!forked) {
        *forked = root;
        (*forked)->h = 10;
        (*forked)->w = 5;
        (*forked)->wd.x = root->wd.x;
        (*forked)->wd.y = root->wd.y;
        (*forked)->wd.w = (int)root->wd.w / 2; // half
        (*forked)->wd.h = root->wd.h;

        other->w = 5;
        other->h = 10;
        other->wd.x = (*forked)->wd.x + (*forked)->wd.w;
        other->wd.y = (*forked)->wd.y;
        other->wd.w = (int)root->wd.w / 2; // half
        other->wd.h = (int)root->wd.h;
    }

    root = NULL;
}
