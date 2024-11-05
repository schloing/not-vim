#include <stdio.h>
#include <sys/stat.h>

#include "nv.h"

int main(int argc, char** argv) {
    assert(argc >= 2);
    struct stat statbuf;
    struct nv_editor editor = { 0 };
    nv_editor_init(&editor);

    char* filename = argv[1];
    FILE* file = fopen(filename, "r+");
    editor->buffers[0] = (nv_buff){
        .name = filename,
        .id   = 0,
        .type = SOURCE,
    };

    fstat(file, statbuf);
    if (S_ISDIR(statbuf.st_mode))
        editor->buffers[0].type = BROWSER;

    nv_load_config(&editor);
    nv_render_editor(&editor);

    fclose(file);
    free(editor->buffers);
    return 0;
}
