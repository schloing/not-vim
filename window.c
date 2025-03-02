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
    window->left = NULL;
    window->parent = NULL;
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
        root->parent = NULL;    // caller sets parent
        root->draw_buffer = false;
        root->draw_children = false;
        root->buffer = NULL;
        return root;
    }

    if (root->draw_buffer) {
        struct nv_window* tmp;
        tmp = nv_find_empty_window(root->left);
        if (tmp) goto set_parent;
        tmp = nv_find_empty_window(root->right);
set_parent:
        if (!tmp) goto exit_statement;
        tmp->parent = root;
        if (!root->left) {
            root->left = tmp;
            root->right = nv_window_init();
            (void)memcpy(root->right, root, sizeof(struct nv_window));
            root->right = root;
        }
        if (!root->right) {
            root->right = tmp;
            root->left = nv_window_init();
            (void)memcpy(root->left, root, sizeof(struct nv_window));
            root->left = root;
        }
        root->draw_buffer = false;
        root->draw_children = true;
    }

exit_statement:
    return NULL;
}

void nv_redistribute(struct nv_window* root)
{

}
