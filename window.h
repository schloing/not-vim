#ifndef NV_WINDOW_H
#define NV_WINDOW_H

struct nv_window;

struct nv_windim {
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
    int padding; // TODO: implement padding
    int descendants;
    bool show;
    bool has_children;
    enum nv_split_kind split;
    struct nv_windim wd;
    struct nv_window* left, *right, *parent;
    // TODO bring nv_buff out of nv_view and into nv_window
    struct nv_view* view; // generally NULL if has_children
};

struct nv_context {
    struct nv_window* window;
    struct nv_view* view;
    struct nv_buff* buffer;
};

#define NV_WD_SET_SIZE(of, _w, _h) \
    do { \
        (of).w = _w; \
        (of).h = _h; \
    } while (0);

struct nv_window* nv_window_init();
void nv_free_windows(struct nv_window* root);
struct nv_window* nv_create_child_window(struct nv_window* root);
int nv_redistribute(struct nv_window* root);

#endif
