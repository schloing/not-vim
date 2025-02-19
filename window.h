#ifndef WINDOW
#define WINDOW

struct nv_window;

struct nv_window {
    int x;
    int y;
    int w;
    int h;
    int padding;
    bool empty;
    size_t buff_id;
    struct nv_window* children[2];
    struct nv_buff buffer;
};

#endif
