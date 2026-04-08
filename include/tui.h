#ifndef NV_TUI_H
#define NV_TUI_H

#include <stdint.h>
#include <cvector.h>

/* ASCII key constants (`tb_event.key`) */
#define TB_KEY_CTRL_TILDE 0x00
#define TB_KEY_CTRL_2 0x00 // clash with `CTRL_TILDE`
#define TB_KEY_CTRL_A 0x01
#define TB_KEY_CTRL_B 0x02
#define TB_KEY_CTRL_C 0x03
#define TB_KEY_CTRL_D 0x04
#define TB_KEY_CTRL_E 0x05
#define TB_KEY_CTRL_F 0x06
#define TB_KEY_CTRL_G 0x07
#define TB_KEY_BACKSPACE 0x08
#define TB_KEY_CTRL_H 0x08 // clash with `CTRL_BACKSPACE`
#define TB_KEY_TAB 0x09
#define TB_KEY_CTRL_I 0x09 // clash with `TAB`
#define TB_KEY_CTRL_J 0x0a
#define TB_KEY_CTRL_K 0x0b
#define TB_KEY_CTRL_L 0x0c
#define TB_KEY_ENTER 0x0d
#define TB_KEY_CTRL_M 0x0d // clash with `ENTER`
#define TB_KEY_CTRL_N 0x0e
#define TB_KEY_CTRL_O 0x0f
#define TB_KEY_CTRL_P 0x10
#define TB_KEY_CTRL_Q 0x11
#define TB_KEY_CTRL_R 0x12
#define TB_KEY_CTRL_S 0x13
#define TB_KEY_CTRL_T 0x14
#define TB_KEY_CTRL_U 0x15
#define TB_KEY_CTRL_V 0x16
#define TB_KEY_CTRL_W 0x17
#define TB_KEY_CTRL_X 0x18
#define TB_KEY_CTRL_Y 0x19
#define TB_KEY_CTRL_Z 0x1a
#define TB_KEY_ESC 0x1b
#define TB_KEY_CTRL_LSQ_BRACKET 0x1b // clash with 'ESC'
#define TB_KEY_CTRL_3 0x1b // clash with 'ESC'
#define TB_KEY_CTRL_4 0x1c
#define TB_KEY_CTRL_BACKSLASH 0x1c // clash with 'CTRL_4'
#define TB_KEY_CTRL_5 0x1d
#define TB_KEY_CTRL_RSQ_BRACKET 0x1d // clash with 'CTRL_5'
#define TB_KEY_CTRL_6 0x1e
#define TB_KEY_CTRL_7 0x1f
#define TB_KEY_CTRL_SLASH 0x1f // clash with 'CTRL_7'
#define TB_KEY_CTRL_UNDERSCORE 0x1f // clash with 'CTRL_7'
#define TB_KEY_SPACE 0x20
#define TB_KEY_BACKSPACE2 0x7f
#define TB_KEY_CTRL_8 0x7f // clash with 'BACKSPACE2'

#define tb_key_i(i) (0xffff - (i))
/* Terminal-dependent key constants (`tb_event.key`) and terminfo caps */
/* BEGIN codegen h */
/* Produced by ./codegen.sh on Tue, 03 Sep 2024 04:17:47 +0000 */
#define TB_KEY_F1 (0xffff - 0)
#define TB_KEY_F2 (0xffff - 1)
#define TB_KEY_F3 (0xffff - 2)
#define TB_KEY_F4 (0xffff - 3)
#define TB_KEY_F5 (0xffff - 4)
#define TB_KEY_F6 (0xffff - 5)
#define TB_KEY_F7 (0xffff - 6)
#define TB_KEY_F8 (0xffff - 7)
#define TB_KEY_F9 (0xffff - 8)
#define TB_KEY_F10 (0xffff - 9)
#define TB_KEY_F11 (0xffff - 10)
#define TB_KEY_F12 (0xffff - 11)
#define TB_KEY_INSERT (0xffff - 12)
#define TB_KEY_DELETE (0xffff - 13)
#define TB_KEY_HOME (0xffff - 14)
#define TB_KEY_END (0xffff - 15)
#define TB_KEY_PGUP (0xffff - 16)
#define TB_KEY_PGDN (0xffff - 17)
#define TB_KEY_ARROW_UP (0xffff - 18)
#define TB_KEY_ARROW_DOWN (0xffff - 19)
#define TB_KEY_ARROW_LEFT (0xffff - 20)
#define TB_KEY_ARROW_RIGHT (0xffff - 21)
#define TB_KEY_BACK_TAB (0xffff - 22)
#define TB_KEY_MOUSE_LEFT (0xffff - 23)
#define TB_KEY_MOUSE_RIGHT (0xffff - 24)
#define TB_KEY_MOUSE_MIDDLE (0xffff - 25)
#define TB_KEY_MOUSE_RELEASE (0xffff - 26)
#define TB_KEY_MOUSE_WHEEL_UP (0xffff - 27)
#define TB_KEY_MOUSE_WHEEL_DOWN (0xffff - 28)

#define TB_CAP_F1 0
#define TB_CAP_F2 1
#define TB_CAP_F3 2
#define TB_CAP_F4 3
#define TB_CAP_F5 4
#define TB_CAP_F6 5
#define TB_CAP_F7 6
#define TB_CAP_F8 7
#define TB_CAP_F9 8
#define TB_CAP_F10 9
#define TB_CAP_F11 10
#define TB_CAP_F12 11
#define TB_CAP_INSERT 12
#define TB_CAP_DELETE 13
#define TB_CAP_HOME 14
#define TB_CAP_END 15
#define TB_CAP_PGUP 16
#define TB_CAP_PGDN 17
#define TB_CAP_ARROW_UP 18
#define TB_CAP_ARROW_DOWN 19
#define TB_CAP_ARROW_LEFT 20
#define TB_CAP_ARROW_RIGHT 21
#define TB_CAP_BACK_TAB 22
#define TB_CAP__COUNT_KEYS 23
#define TB_CAP_ENTER_CA 23
#define TB_CAP_EXIT_CA 24
#define TB_CAP_SHOW_CURSOR 25
#define TB_CAP_HIDE_CURSOR 26
#define TB_CAP_CLEAR_SCREEN 27
#define TB_CAP_SGR0 28
#define TB_CAP_UNDERLINE 29
#define TB_CAP_BOLD 30
#define TB_CAP_BLINK 31
#define TB_CAP_ITALIC 32
#define TB_CAP_REVERSE 33
#define TB_CAP_ENTER_KEYPAD 34
#define TB_CAP_EXIT_KEYPAD 35
#define TB_CAP_DIM 36
#define TB_CAP_INVISIBLE 37
#define TB_CAP__COUNT 38
/* END codegen h */

/* Some hard-coded caps */
#define TB_HARDCAP_ENTER_MOUSE "\x1b[?1000h\x1b[?1002h\x1b[?1015h\x1b[?1006h"
#define TB_HARDCAP_EXIT_MOUSE "\x1b[?1006l\x1b[?1015l\x1b[?1002l\x1b[?1000l"
#define TB_HARDCAP_STRIKEOUT "\x1b[9m"
#define TB_HARDCAP_UNDERLINE_2 "\x1b[21m"
#define TB_HARDCAP_OVERLINE "\x1b[53m"

/* Colors (numeric) and attributes (bitwise) (`tb_cell.fg`, `tb_cell.bg`) */
#define TB_DEFAULT 39
#define TB_BLACK   30
#define TB_RED     31
#define TB_GREEN   32
#define TB_YELLOW  33
#define TB_BLUE    34
#define TB_MAGENTA 35
#define TB_CYAN    36
#define TB_WHITE   37

#define TB_BG_DEFAULT 49
#define TB_BG_BLACK   40
#define TB_BG_RED     41
#define TB_BG_GREEN   42
#define TB_BG_YELLOW  43
#define TB_BG_BLUE    44
#define TB_BG_MAGENTA 45
#define TB_BG_CYAN    46
#define TB_BG_WHITE   47

enum {
    NV_TUI_HL_WHITE_ON_BLACK = 0,
    NV_TUI_HL_BLACK_ON_WHITE = 1,
};

struct nv_hl {
    uint32_t fg;
    uint32_t bg;
    uint16_t attr;
};

// hl_index is an index into nv_hls
typedef uint32_t hl_index;

struct nv_tui_cell {
    uint32_t rune;
    hl_index hl;
};

int nv_tui_init();
void nv_tui_free();
void nv_tui_clear();
void nv_tui_present();
void nv_tui_set_cell(int x, int y, uint32_t ch, hl_index hl);
void nv_tui_printf(int x, int y, hl_index hl, const char* fmt, ...);
size_t nv_tui_width();
size_t nv_tui_height();

#endif