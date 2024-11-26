#include <stdio.h>
#include "events.h"
#include "plugin.h"
#include "termbox2.h"

void main(void) {
    printf("hello from plugload\n");
    tb_print(0, 0, TB_WHITE, TB_BLACK, "plugload");
}

struct nv_plugin _NV_PLUGIN_DESCRIPTOR = {
    .name = "plugload",
    .author = "jonkler",
    .iteration = 0,
    .subs = 0,
    .main = main,
};
