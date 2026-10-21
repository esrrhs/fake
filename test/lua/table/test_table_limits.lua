-- Integer arguments must use the same checked native implementation on every
-- backend; these cases guard against reintroducing stale JIT-only copies.

function test_table_move_over_cap()
    local src = {}
    src[1] = 11
    src[10000001] = 22
    local dst = {}
    dst[1] = 1
    dst[2] = 2
    dst[10000001] = 3
    local ret = table.move(src, 1, 10000001, 1, dst)
    if ret ~= dst then return 1 end
    if dst[1] ~= 11 then return 2 end
    if dst[2] ~= nil then return 3 end
    if dst[10000001] ~= 22 then return 4 end
    return 5000
end

function test_table_move_src_wrap()
    table.move({}, math.mininteger, math.maxinteger, 1, {})
    return 1
end

function test_table_move_src_overflow()
    -- 0 .. maxinteger 的个数是 2^63，装不进 int64
    table.move({}, 0, math.maxinteger, 1, {})
    return 1
end

function test_table_move_dst_wrap()
    -- 源区间只有 2 个元素，终点越过 maxinteger
    table.move({ 1, 2 }, 1, 2, math.maxinteger, {})
    return 1
end

function test_table_unpack_over(n)
    table.unpack({}, 1, n)
    return 1
end

function test_table_unpack_wrap()
    table.unpack({}, math.mininteger, math.maxinteger)
    return 1
end

function test_table_unpack_ok()
    local t = {}
    t[1] = 4
    t[3] = 6
    local a, b, c = table.unpack(t, 1, 3)
    if a ~= 4 or b ~= nil or c ~= 6 then return 0 end
    local x = table.unpack(t, math.maxinteger, math.maxinteger)
    if x ~= nil then return 0 end
    t[math.maxinteger] = 8
    x = table.unpack(t, math.maxinteger, math.maxinteger)
    if x ~= 8 then return 0 end
    return 5000
end
