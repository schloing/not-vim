#ifndef NV_PIECE_H
#define NV_PIECE_H

#include "error.h"

struct nv_piece {
    char* buffer;
    size_t offset;
    size_t length;
};

struct nv_rope_node;

struct nv_rope_node {
    struct nv_rope_node* parent, *left, *right;
    size_t offset;
    int char_count;
    int line_count;
};

#define NV_ROPE_LEAF_SIZE 256

int nv_rope_init(struct nv_rope_node* rope, const char* buffer, size_t length)
{
    if (length < NV_ROPE_LEAF_SIZE) {
        return 0;
    }

    rope->offset = length / 2 - NV_ROPE_LEAF_SIZE;

    if (length / 2 < NV_ROPE_LEAF_SIZE) {
        rope->offset = 0;
    }

    rope->left = (struct nv_rope_node*)malloc(sizeof(struct nv_rope_node));
    rope->right = (struct nv_rope_node*)malloc(sizeof(struct nv_rope_node));

    nv_rope_init(rope->left, buffer - NV_ROPE_LEAF_SIZE, length - NV_ROPE_LEAF_SIZE);
    nv_rope_init(rope->right, buffer + NV_ROPE_LEAF_SIZE, length + NV_ROPE_LEAF_SIZE);
}

// https://dev.to/_darrenburns/the-piece-table---the-unsung-hero-of-your-text-editor-al8
// https://en.wikipedia.org/wiki/Piece_table
// https://stackoverflow.com/questions/8371556/line-management-in-a-text-editor

#endif
