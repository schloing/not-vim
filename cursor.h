#ifndef CURSOR_H
#define CURSOR_H

struct cursor {
    int x;
    int y;
    int line;
    char ch;
    size_t buff_id;
};

#define NV_CURSOR_CAP       5
#define NV_PRIMARY_CURSOR   0

#endif
