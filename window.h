#ifndef WINDOW
#define WINDOW

struct nv_window;

struct nv_window_desc {
    int x;
    int y;
    int w;
    int h;
};

struct nv_window {
    int padding;
    bool draw_buffer;
    bool draw_children;
    struct nv_window_desc wd;
    struct nv_window* parent;
    struct nv_window* left;
    struct nv_window* right;
    struct nv_buff* buffer;
};

struct nv_window* nv_window_init();
void nv_free_windows(struct nv_window* root);
struct nv_window* nv_find_empty_window(struct nv_window* root);
void nv_redistribute(struct nv_window* root);

#endif
