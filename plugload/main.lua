function buff_redraw_handler(ctx)
    local buff = ctx:get_buffer()
    echo(tostring(buff:path()) .. "\n")
end
local id = event.subscribe(event.BUFFDRAW, buff_redraw_handler)