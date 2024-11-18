#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "termbox2.h"
#include "vec.h"
#include "plugin.h"

static void load_plugload(struct nv_editor* editor) {
    struct nv_buff logbuff = { .path = "logs", .type = NV_BUFFTYPE_PLAINTEXT };
    nv_buffer_init(&logbuff, NULL);
    nv_push_buffer(editor, logbuff);

    void* handle = dlopen("./plugload.so", RTLD_NOW | RTLD_GLOBAL);

    if (!handle) {
        // plugload.so not found?
        sprintf(logbuff.buffer, "plugin load failed: %s\n", dlerror());
        return;
    }

    struct nv_plugin* plugload = (struct nv_plugin*)dlsym(handle, "_NV_PLUGIN_DESCRIPTOR");

    if (!plugload) {
        // _NV_PLUGIN_DESCRIPTOR wasn't defined
        sprintf(logbuff.buffer, "could not find symbol _NV_PLUGIN_DESCRIPTOR: %s\n", dlerror());
        goto call_dlclose;
    }

    sprintf(logbuff.buffer, "successfully loaded %s: %s v%d\n",
            plugload->author, plugload->name, plugload->iteration);

//  plugload->main();

call_dlclose:
    dlclose(handle);
}

int main(int argc, char** argv) {
    int rv = 0;
    assert(argc >= 2);
    struct nv_editor editor = { 0 };
    nv_editor_init(&editor);

    if ((rv = tb_init()) != TB_OK) {
        fprintf(stderr, "%s\n", tb_strerror(rv));
        editor.status = rv;
        return rv;
    }

    struct nv_buff buff = { .id = 0 };
    nv_buffer_init(&buff, argv[1]);
    nv_push_buffer(&editor, buff);

    editor.width = tb_width();
    editor.height = tb_height();

    load_plugload(&editor);
    editor.peek = 1;
    nv_mainloop(&editor);

    tb_shutdown();
    nv_free_buffers(&editor);
    return rv;
}
