-- select 与 Lua 5.4 对齐：
--   0 和过小的负下标报 index out of range
--   下标超过实参个数返回 0 个值，不是一个 nil
--   1.5 不能截成整数
local function must_err(fn, needle)
    local ok, err = pcall(fn)
    if ok then return false end
    if needle and not string.find(tostring(err), needle, 1, true) then return false end
    return true
end

local function nvals(...)
    return select("#", ...)
end

function test_basic_select_range()
    if not must_err(function() select(0, 10, 20) end, "index out of range") then return 1 end
    if not must_err(function() select(-3, 10, 20) end, "index out of range") then return 2 end
    if not must_err(function() select(math.mininteger, 10) end, "index out of range") then return 3 end
    if not must_err(function() select(1.5, 10, 20) end, "integer") then return 4 end

    if nvals(select(5, 1, 2, 3)) ~= 0 then return 5 end
    if nvals(select(5, 10)) ~= 0 then return 6 end

    local a, b = select(2, 10, 20, 30)
    if a ~= 20 or b ~= 30 then return 7 end
    if select(-1, 10, 20, 30) ~= 30 then return 8 end
    if select("#", 10, 20, 30) ~= 3 then return 9 end
    if select("#", nil) ~= 1 then return 10 end
    if select("#", string.byte("abc", 5, 4)) ~= 0 then return 11 end

    return 5000
end
