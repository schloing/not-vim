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

static struct nv_window* nv_assign_child(struct nv_window* root, struct nv_window** child)
{
    *child = nv_find_empty_window(*child);

    if (*child) {
        (*child)->parent = root;
        return *child;
    }

    return NULL;
}

struct nv_window* nv_find_empty_window(struct nv_window* root)
{
    if (!root) {
        root = nv_window_init();
        if (!root) {
            return NULL;
        }
        root->buffer = NULL;
        root->parent = NULL;
        root->active = true;
        return root;
    }

    if (!root->active) { // root window is allocated but not active
        root->active = true;
        return root;
    }

    return nv_assign_child(root, &root->left) ? root->left : nv_assign_child(root, &root->right);
}

void nv_redistribute(struct nv_window* root)
{
    if (!root) {
        return;
    }
    struct nv_window* new = nv_window_init();
    if (!new) {
        return;
    }
    (void)memcpy(new, root, sizeof(struct nv_window));
    struct nv_window* other = NULL;

    if (!root->left) {
        other = root->right;
    }

    if (!root->right) {
        other = root->left;
    }

    if (root->active) {
        new->w = 5;
        new->h = 10;
        new->wd.x = root->wd.x;
        new->wd.y = root->wd.y;
        new->wd.w = root->wd.w / 2; // half
        new->wd.h = root->wd.h;

        other->w = 5;
        other->h = 10;
        other->wd.x = new->wd.x + new->wd.w;
        other->wd.y = new->wd.y;
        other->wd.w = root->wd.w - new->wd.w; // also half
        other->wd.h = root->wd.h;

        root->left = other;
        root->right = new;
        root->buffer = NULL; // root->buffer copied to root->right
                             // not setting to NULL leads to double free
        root->active = false;
    }

    root = NULL;
}
