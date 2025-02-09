#include "editor.h"
#include "termbox2.h"
#include "window.h"

void nv_open_window(struct nv_editor* editor, struct nv_window window) {
    if (!editor) return;
    cvector_push_back(editor->windows, window);
    size_t amt_windows = cvector_size(editor->windows) + 1;
    size_t w = tb_width() / amt_windows;
    // TODO: maintain current config of windows
    for (size_t i = 0; i < amt_windows; i++) {
        editor->windows[i].x = i * w;
        editor->windows[i].w = w;
        editor->windows[i].y = 0;
        editor->windows[i].h = tb_height() - 3; // FIXME
        editor->windows[i].padding = 1;
    }
}
