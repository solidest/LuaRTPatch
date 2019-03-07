
--
-- tests for GC activation when creating different kinds of objects
--
local function GC1 ()
  local i = 0
  local u
  local b     -- (above 'u' it in the stack)
  local finish = false
  u = setmetatable({}, {__gc = function () finish = true end})
  b = {34}
  repeat 
    u = {} 
    i = i+ 1
  until finish
  assert(b[1] == 34)   -- 'u' was collected, but 'b' was not

  finish = false; local i = 1
  u = setmetatable({}, {__gc = function () finish = true end})
  repeat i = i + 1; u = tostring(i) .. tostring(i) until finish
  assert(b[1] == 34)   -- 'u' was collected, but 'b' was not

  finish = false
  u = setmetatable({}, {__gc = function () finish = true end})
  repeat local i; u = function () return i end until finish
  assert(b[1] == 34)   -- 'u' was collected, but 'b' was not
end

local function GC2 ()
  local i = 0
  local u
  local finish = false
  u = {setmetatable({}, {__gc = function () finish = true end})}
  local b = {34}
  repeat u = {{}} until finish
  assert(b[1] == 34)   -- 'u' was collected, but 'b' was not

  finish = false; local i = 1
  u = {setmetatable({}, {__gc = function () finish = true end})}
  repeat i = i + 1; u = {tostring(i) .. tostring(i)} until finish
  assert(b[1] == 34)   -- 'u' was collected, but 'b' was not

  finish = false
  u = {setmetatable({}, {__gc = function () finish = true end})}
  repeat local i; u = {function () return i end} until finish
  assert(b[1] == 34)   -- 'u' was collected, but 'b' was not
end

-- set_rtgc(3, 0.2)
-- count = 0
-- while(true) do
-- 	print "start"
-- 	--GC1()
-- 	--GC2()
-- 	GC0(1)
-- 	count = count + 1
-- 	local ss = "count="..tostring(count)
-- 	print(ss)
-- end


local function GC ()
  GC1()
  GC2()
end

GC()

-- set_rtgc(0.6, 0.4)
-- i = 0
-- repeat
--   res, msg = pcall(GC)
--   i = i + 1
--   print("pass"..tostring(i))
-- until not res
-- print(msg)

  
  
