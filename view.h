#ifndef NV_VIEW_H
#define NV_VIEW_H

#include <stddef.h>
#include <stdbool.h>

#include "cvector.h"
#include "cursor.h"

struct nv_view {
    size_t top_line_index;
    size_t padding;
    size_t gutter_gap;
    size_t gutter_width_cols;
    bool visible;
    bool allow_split;
    struct nv_buff* buffer;
    cvector(struct cursor) cursors;
};

#endif