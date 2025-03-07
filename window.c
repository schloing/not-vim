#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "editor.h"
#include "termbox2.h"
#include "window.h"

struct nv_window* nv_window_init()
{
    struct nv_window* window = (struct nv_window*)calloc(1, sizeof(struct nv_window));
    window->buffer = NULL;
    window->draw_buffer = true;
    window->draw_children = false;
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
        root->draw_buffer = true;
        root->draw_children = false;
        root->buffer = NULL;
        return root;
    }

    if (root->draw_buffer && !root->buffer) {
        // first caller has to call nv_buffer_init
        root->buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));
        return root;
    }

    if (!root->draw_children) {
        root->left = nv_find_empty_window(root->left);

        if (!root->left) {
            return NULL;
        }

        root->left->parent = root;

        if (root->draw_buffer && root->buffer) {
            root->right = nv_window_init();
            (void)memcpy(root->right, root, sizeof(struct nv_window));
            root->right->buffer = root->buffer;
            root->right->parent = root;
            root->right->left = NULL;
            root->right->right = NULL;
            root->right->draw_buffer = true;
            root->right->draw_children = false;
        }

        root->draw_children = true;
        root->draw_buffer = false;
        root->buffer = NULL;
        return root->left;
    }

    return nv_find_empty_window(root->right);
}

void nv_redistribute(struct nv_window* root)
{
    if (!root) {
        return;
    }

    if (root->right && root->left) {
        root->left->wd.x = root->wd.x;
        root->left->wd.w = root->wd.w / 2;
        root->left->wd.h = root->wd.h;
        root->right->wd.x = root->wd.x + root->left->wd.w;
        root->right->wd.w = root->wd.w - root->left->wd.w;
        root->right->wd.h = root->wd.h;
        nv_redistribute(root->right);
        nv_redistribute(root->left);
    }
}
