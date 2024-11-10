#ifndef ASSERT_H
#define ASSERT_H

#define NV_ASSERT(expression) \
    do {        \
        if (!expression) \
            fprintf(stderr, "assertion failed %s in %s:%d", __func__, __FILE__, __LINE__); \
    } while (0)

#endif
