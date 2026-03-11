-- providers wrappers for c functions, communicate them over the nng rpc
local ffi = require("ffi")

ffi.cdef[[
typedef struct {
    int      tab_width;
    bool     expand_tab;
    bool     auto_indent;
    bool     line_numbers;
    bool     show_relative;
    int      command_delay;
    int      status_height;
    bool     show_status;
    bool     show_buffer;
    bool     show_headless;
    bool     allow_italic_text;
    bool     allow_bolded_text;
    bool     allow_underlined_text;
    uint32_t fg_main;
    uint32_t fg_secondary;
    uint32_t fg_contrast;
    uint32_t bg_main;
    uint32_t bg_secondary;
    uint32_t bg_contrast;
    uint32_t fg_syntax_keyword;
    uint32_t fg_syntax_string;
    uint32_t fg_syntax_comment;
    uint32_t fg_syntax_number;
    uint32_t fg_syntax_operator;
    uint32_t fg_syntax_function;
    uint32_t fg_syntax_variable;
    uint32_t fg_syntax_type;
    uint32_t fg_syntax_macro;
    uint32_t fg_syntax_preprocessor;
    uint32_t fg_syntax_constant;
    uint32_t fg_syntax_builtins;
    uint32_t fg_syntax_attribute;
    uint32_t fg_syntax_error;
} nv_conf;
]]

if not config then
    print("config null\n")
    return nil
end

local nvconf = ffi.cast("nv_conf*", config)

if nvconf == nil then
    print("cast failed\n")
    return nil
end

local editor = { config = nvconf }

return editor