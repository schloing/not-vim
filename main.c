#include <stdio.h>
#include <sys/stat.h>

#include "nv.h"

int main(int argc, char** argv) {
    assert(argc >= 2);
    struct nv_editor editor = { 0 };
    struct stat statbuf;
    editor_init(&editor);

    char* filename = argv[1];
    FILE* file = fopen(filename, "w");
    fstat(file, statbuf);

    if (S_ISDIR(statbuf.st_mode)) {
        
    }
    else {
    }

    fclose(file);
    free(editor->buffers);
    return 0;
}
