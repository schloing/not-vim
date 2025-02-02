#include <stdio.h>
#include <stdlib.h>
#include "././events.h"
#include "././plugin.h"
#include "././termbox2.h"

static void plugload_open_window() {
    tb_print(1, 1, TB_256_BLACK, 255, "plugload_open_window");
}

void main(void) {
    nv_register_event(NV_EVENT_HOTKEY, "plugload_open_window", plugload_open_window);
}

struct nv_plugin _NV_PLUGIN_DESCRIPTOR = {
    .name = "plugload",
    .author = "jonkler",
    .description = "a plugin manager",
    .iteration = 0,
    .subs = NV_EVENT_HOTKEY,
    .main = main,
};
