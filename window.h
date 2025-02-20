#ifndef WINDOW
#define WINDOW

struct nv_window;

struct nv_window {
    int x;
    int y;
    int w;
    int h;
    int padding;
    bool active; // !active means buffer is allocated
                 // active means either left or right .active
    struct nv_window* left;
    struct nv_window* right;
    struct nv_buff* buffer;
};

struct nv_window* nv_window_init();
void nv_free_windows(struct nv_window* root);
struct nv_window* nv_find_empty_window(struct nv_window* root);

#endif
