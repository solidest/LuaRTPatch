


function testall(gctime)

    set_rtgc(gctime, 1)

    --dofile("rttest_lua_gc.lua")
    dofile("goto.lua")
    dofile("locals.lua")
    --dofile("main.lua")
    dofile("math.lua")
    dofile("nextvar.lua")
    dofile("pm.lua")

    dofile("strings.lua")
    dofile("tpack.lua")
    dofile("utf8.lua")
    --dofile("vararg.lua")
    dofile("verybig.lua")

    dofile("sort.lua")
    dofile("heavy.lua")

    set_rtgc(0, 0)

end



for i = 1, 9 do

    testall(0.1*i)
    --set_rtgc(gct*0.1, 1-gct*0.1)
    --dofile("math.lua")

end


