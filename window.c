#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "editor.h"
#include "error.h"
#include "termbox2.h"
#include "window.h"

static void nv_set_width_and_split(struct nv_window* root, struct nv_window* left, struct nv_window* right);

struct nv_window* nv_window_init()
{
    struct nv_window* window = (struct nv_window*)calloc(1, sizeof(struct nv_window));

    if (!window) {
        nv_editor->status = NV_ERR_MEM;
        return NULL;
    }

    window->has_children = false;
    window->left = NULL;
    window->parent = NULL;
    window->right = NULL;
    window->show = true;
    window->view = NULL;
    
    return window;
}

void nv_free_windows(struct nv_window* root)
{
    if (!root) {
        return;
    }

    nv_free_windows(root->left);
    nv_free_windows(root->right);
    nv_free_view(root->view);

    free(root);
}

// creates a child window wherever it finds a NULL child, walking down root
struct nv_window* nv_create_child_window(struct nv_window* root)
{
    if (!root) {
        root = nv_window_init();

        if (nv_editor->status != NV_OK) {
            return NULL;
        }

        root->has_children = false;
        root->parent = NULL; // caller sets parent
        root->show = true;
        root->view = NULL;

        nv_editor->focus = root;
        
        return root;
    }

    if (!root->has_children && !root->view) {
        nv_editor->focus = root;
        return root;
    }

    if (root->show && !root->has_children) {
        root->left = nv_create_child_window(root->left);

        if (nv_editor->status != NV_OK) {
            return NULL;
        }

        root->left->parent = root;

        if (root->view->buffer) {
            root->right = nv_window_init();

            if (nv_editor->status != NV_OK) {
                return NULL;
            }

            memcpy(root->right, root, sizeof(struct nv_window));
            root->right->view = root->view;
            root->right->parent = root;
            root->right->left = NULL;
            root->right->right = NULL;
            root->right->show = true;
            root->right->has_children = false;
            root->right->split = root->split == HORIZONTAL ? VERTICAL : HORIZONTAL;
        }

        root->has_children = true;
        root->view = NULL;
        root->descendants += 2;

        nv_editor->focus = root->left;

        return root->left;
    }

    struct nv_window* right = nv_create_child_window(root->right);

    if (nv_editor->status != NV_OK) {
        return NULL;
    }

    if (right) {
        right->parent = root;
    }

    nv_editor->focus = right;

    return right;
}

static void nv_set_width_and_split(struct nv_window* root, struct nv_window* left, struct nv_window* right)
{
    enum nv_split_kind split = HORIZONTAL;

    if (root->parent) {
        split = root->parent->split == HORIZONTAL ? VERTICAL : HORIZONTAL;
    }

    if (left->show) {
        left->wd.x = root->wd.x;
        left->wd.y = root->wd.y;
        left->wd.w = split == HORIZONTAL ? root->wd.w / 2 : root->wd.w;
        left->wd.h = split == VERTICAL ? root->wd.h / 2 : root->wd.h;
    }

    if (right->show) {
        right->wd.x = split == HORIZONTAL ? root->wd.x + left->wd.w : root->wd.x;
        right->wd.y = split == VERTICAL ? root->wd.y + left->wd.h : root->wd.y;
        right->wd.w = split == HORIZONTAL ? root->wd.w - left->wd.w : root->wd.w;
        right->wd.h = split == VERTICAL ? root->wd.h - left->wd.h : root->wd.h;
    }
}

int nv_redistribute(struct nv_window* root)
{
    if (!root) {
        return NV_ERR_NOT_INIT;
    }

    if (root->right && root->left) {
        nv_set_width_and_split(root, root->left, root->right);
        (void)nv_redistribute(root->right);
        (void)nv_redistribute(root->left);
        return NV_OK;
    }

    struct nv_window* window = root->right == NULL ? root->left : root->right;
    if (!window) {
        // FIXME: NV_ERR or NV_ERR_NOT_INIT?
        return NV_ERR_NOT_INIT;
    }

    window->wd.x = root->wd.x;
    window->wd.y = root->wd.y;
    window->wd.w = root->wd.w;
    window->wd.h = root->wd.h;

    return NV_OK;
}
