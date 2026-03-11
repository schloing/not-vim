local chunk, err = loadfile("./lua/binder/notvim.lua")
assert(chunk, err)
local editor = chunk()

function buff_redraw_handler(ctx)
	-- executes once per editor global redraw
end
local id = event.subscribe(event.BUFFDRAW, buff_redraw_handler)