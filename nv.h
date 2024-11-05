#ifndef NV_H
#define NV_H

#include <stdbool.h>

#define NV_TAB_WIDTH     4
#define NV_EXPAND_TAB    true
#define NV_AUTO_INDENT   true
#define NV_LINE_NUMBERS  true
#define NV_SHOW_RELATIVE true
#define NV_COMMAND_DELAY 350

typedef int nv_bufftype;
enum nv_bufftypes {
    NV_STDIN    = 1,
    NV_STDOUT   = 2,
    BROWSER     = 4,
    NETWORK     = 8,
    SOURCE      = 16,
};

typedef struct {
    char*       name;
    size_t      id;
    nv_bufftype type;
} nv_buff;

struct nv_editor {
    nv_buff* buffers;

    struct nv_conf {
         int  tab_width;
         bool expand_tab;
         bool auto_indent;
         bool line_numbers;
         bool show_relative;
         int  command_delay;
    } nv_conf;
};

#endif
