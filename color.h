#ifndef NV_COLOR_H
#define NV_COLOR_H

#include "termbox2.h"

#if TB_OPT_ATTR_W >= 32
#define NV_TRUECOLOR
#endif

#define NV_DEFAULT  TB_DEFAULT

#ifdef NV_TRUECOLOR
#define NV_BLACK    TB_HI_BLACK
#define NV_RED      0xff0000
#define NV_GREEN    0x00ff00
#define NV_YELLOW   0xffff00
#define NV_BLUE     0x0000ff
#define NV_MAGENTA  0xff00ff
#define NV_CYAN     0x00ffff
#define NV_WHITE    0xffffff
#else
#define NV_BLACK    TB_BLACK
#define NV_RED      TB_RED
#define NV_GREEN    TB_GREEN
#define NV_YELLOW   TB_YELLOW
#define NV_BLUE     TB_BLUE
#define NV_MAGENTA  TB_MAGENTA
#define NV_CYAN     TB_CYAN
#define NV_WHITE    TB_WHITE
#endif

#endif
