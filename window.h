#ifndef NV_WINDOW_H
#define NV_WINDOW_H

struct nv_window;

struct nv_window_desc {
    int x;
    int y;
    int w;
    int h;
};

enum nv_split_kind {
    NONE,
    HORIZONTAL,
    VERTICAL,
};

struct nv_window {
    int padding;
    bool show;
    bool has_children;
    size_t descendants;
    enum nv_split_kind split;
    struct nv_window_desc wd;
    struct nv_window* parent;
    struct nv_window* left;
    struct nv_window* right;
    struct nv_buff* buffer; // NULL if has_children
};

struct nv_window* nv_window_init();
void nv_free_windows(struct nv_window* root);
struct nv_window* nv_find_empty_window(struct nv_window* root);
int nv_redistribute(struct nv_window* root);

#endif
