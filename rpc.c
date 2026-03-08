#include "rpc.h"
#include "error.h"
#include "events.h"
#include "yyjson.h"

int nv_rpc_parse(const char* buffer, size_t bufsiz)
{
    yyjson_doc* doc = yyjson_read(buffer, bufsiz, 0);
    if (!doc) {
        return NV_ERR;
    }

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_obj(root)) {
        yyjson_doc_free(doc);
        return NV_ERR;
    }

    yyjson_val* v_jsonrpc = yyjson_obj_get(root, "jsonrpc");
    yyjson_val* v_method  = yyjson_obj_get(root, "method");
    yyjson_val* v_params  = yyjson_obj_get(root, "params");
    yyjson_val* v_id      = yyjson_obj_get(root, "id");

    if (!v_jsonrpc
            || !yyjson_is_str(v_jsonrpc)
            || strcmp(yyjson_get_str(v_jsonrpc), "2.0") != 0
            || !v_method
            || !yyjson_is_str(v_method)) {
        yyjson_doc_free(doc);
        return NV_ERR;
    }
    
    // https://www.jsonrpc.org/specification
    // TODO: determine request/response
    // if request, run requested method with params, schedule a response to be sent
    // https://docs.libuv.org/en/v1.x/async.html#c.uv_async_send
    // if response, handle it

    return NV_OK;
}