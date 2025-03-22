#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "editor.h"
#include "termbox2.h"
#include "window.h"

static void nv_set_widths(struct nv_window* root, struct nv_window* left, struct nv_window* right);

struct nv_window* nv_window_init()
{
    struct nv_window* window = (struct nv_window*)calloc(1, sizeof(struct nv_window));
    window->buffer = NULL;
    window->show = true;
    window->has_children = false;
    window->parent = NULL;
    window->left = NULL;
    window->right = NULL;
    return window;
}

void nv_free_windows(struct nv_window* root)
{
    if (!root) {
        return;
    }

    nv_free_buffer(root->buffer);
    root->buffer = NULL;
    nv_free_windows(root->left);
    nv_free_windows(root->right);

    free(root);
    root = NULL;
}

struct nv_window* nv_find_empty_window(struct nv_window* root)
{
    if (!root) {
        root = nv_window_init();
        root->parent = NULL; // caller sets parent
        root->show = true;
        root->has_children = false;
        root->buffer = NULL;
        return root;
    }

    if (root->show && !root->buffer) {
        // first caller has to call nv_buffer_init
        root->buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));
        return root;
    }

    if (!root->has_children) {
        root->left = nv_find_empty_window(root->left);

        if (!root->left) {
            return NULL;
        }

        root->left->parent = root;

        if (root->show && root->buffer) {
            root->right = nv_window_init();
            memcpy(root->right, root, sizeof(struct nv_window));
            root->right->buffer = root->buffer;
            root->right->parent = root;
            root->right->left = NULL;
            root->right->right = NULL;
            root->right->show = true;
            root->right->has_children = false;
        }

        root->has_children = true;
        root->show = false;
        root->buffer = NULL;
        root->descendants += 2;

        return root->left;
    }

    struct nv_window* right = nv_find_empty_window(root->right);

    if (right) {
        right->parent = root;
    }

    return right;
}

static void nv_set_widths(struct nv_window* root, struct nv_window* left, struct nv_window* right)
{
    bool left_block = false;

    if (left->show && !left->has_children) {
        left->wd.x = root->wd.x;
        left->wd.w = root->wd.w / 2;
        left->wd.h = root->wd.h;
        left_block = true;
    }

    if (right->show && !right->has_children) {
        right->wd.x = left_block ? root->wd.x + left->wd.w : root->wd.x;
        right->wd.w = left_block ? root->wd.w - left->wd.w : root->wd.w;
        right->wd.h = root->wd.h;
    }
}

void nv_redistribute(struct nv_window* root)
{
    if (!root) {
        return;
    }

    if (root->right && root->left) {
        nv_set_widths(root, root->left, root->right);
        nv_redistribute(root->right);
        nv_redistribute(root->left);
        return;
    }

    struct nv_window* window = root->right == NULL ? root->left : root->right;

    if (!window) {
        return;
    }

    window->wd.x = root->wd.x;
    window->wd.y = root->wd.y;
    window->wd.w = root->wd.w;
    window->wd.h = root->wd.h;
}
