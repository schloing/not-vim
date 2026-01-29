#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "editor.h"
#include "error.h"
#include "termbox2.h"
#include "window.h"

#include <stdlib.h>
#include <stdio.h>

static nv_pool_index nv_split_view(); // returns pool index to parent view
static void layout();

struct nv_window_node* nv_window_node_init()
{
    struct nv_window_node* window = (struct nv_window_node*)malloc(sizeof(struct nv_window_node));

    if (!window) {
        nv_editor->status = NV_ERR_MEM;
        return NULL;
    }

    static size_t id = 0;
    window->id = id++;
    window->kind = NV_WM_VIEW;
    window->view = NULL;

    cvector_push(nv_editor->windows, window);
    nv_editor->status = NV_OK;
    return window;
}

static nv_err nv_window_set_focus(struct nv_window* focus)
{
    if (!focus) {
        return NV_ERR_MEM;
    }

    if (focus->kind != NV_WM_VIEW) {
        // focus->kind MUST be NV_WM_VIEW because a split cannot be split further without making a choice
        return NV_ERR;
    }

    nv_editor->focus = focus;

    return NV_OK;
}

struct nv_window_node* nv_window_node_push_child(struct nv_window_node* root, struct nv_window_node* child)
{
    if (!child || !root) {
        nv_editor->status = NV_ERR_NOT_INIT;
        return NULL;
    }

    assert(root->kind == NV_WM_VIEW);

    struct nv_view* view = root->leaf.view;
    root->leaf.view = NULL;

    root->kind = NV_WM_SPLIT;
    root->split.kind = NV_SPLIT_HORIZONTAL;
    root->split.right = child;
    root->split.ratio = 0.5f;

    root->split.left = nv_window_node_init();
    root->split.left->leaf.view = view;

    return child;
}
