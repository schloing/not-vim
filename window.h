#ifndef WINDOW
#define WINDOW

struct nv_window {
    int x;
    int y;
    int w;
    int h;
    int padding;
    int color;
    size_t buff_id;
    struct nv_buff buff;
};

#endif
