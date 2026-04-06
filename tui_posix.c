#include <termios.h>
#include <unistd.h>
#include <tui.h>
#include <error.h>

// extern'd tui.h
struct nv_hl nv_hls[] = { };
struct termios termios0;

int nv_tui_init()
{
    int rv = NV_OK;
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if ((rv = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw))) {

    }
}

static void nv_tui_deinit()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios0);
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void nv_tui_free()
{
    nv_tui_deinit();
}

void nv_tui_refresh()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}