-- 函数调用实参个数语义（对齐 Lua 5.4）：实参少于形参时缺失形参为 nil，
-- 实参多于形参时多出的实参求值后丢弃。覆盖各条调用路径：
--   1. 同文件 local function 直接调用（CGen 直接调用 / interp CALLNAME）
--   2. 闭包变量调用（FlCallClosure / interp CALL）
--   3. 文件级 function（数学特化直调、FakeluaCallByName 按名分发）
--   4. 多值展开（返回值个数少于/多于形参）
--   5. vararg（固定参数补 nil，多余实参进入 ...）
--   6. 原生函数（多传丢弃）
local function two(a, b)
    return (a or 0) + (b or 0)
end

local function one(a)
    return a or 0
end

local function va(a, ...)
    return (a or 0) * 10 + select("#", ...)
end

local closure_var = two

local function ret1()
    return 7
end

local function ret3()
    return 1, 2, 3
end

-- 文件级函数，会被数学特化
function double_arity(n)
    return n * 2
end

function add2_arity(x, y)
    return x + y
end

function test_basic_call_arity()
    local total = 0

    -- 1. local function 直接调用：少传补 nil，多传丢弃
    total = total + two(1)            -- 1
    total = total + two(1, 2)         -- 3
    total = total + two(1, 2, 100)    -- 3
    total = total + one(5, 6, 7)      -- 5

    -- 2. 闭包变量调用
    total = total + closure_var(4)         -- 4
    total = total + closure_var(4, 5)      -- 9
    total = total + closure_var(4, 5, 100) -- 9

    -- 3. 文件级函数：数学特化路径多传/精确/少传
    total = total + double_arity(3)      -- 6
    total = total + add2_arity(1, 2)     -- 3
    total = total + add2_arity(1, 2, 100)-- 3（多传丢弃，不能生成参数个数不符的 C）
    total = total + add2_arity(1, 2, 100, 200) -- 3

    -- 4. 多值展开：返回值个数少于/多于形参
    total = total + two(ret1())       -- 7 + 0 = 7
    total = total + two(ret3())       -- 1 + 2 = 3
    total = total + one(ret3())       -- 1
    total = total + two(1, ret3())    -- 1 + 1 = 2（ret3 其余返回值丢弃）
    total = total + one(1, ret3())    -- 1（形参已填满，展开值丢弃）

    -- 5. vararg：固定参数补 nil，多余实参进入 ...
    total = total + va(1)             -- 10 + 0 = 10
    total = total + va(1, 2, 3)       -- 10 + 2 = 12
    total = total + va()              -- 0 + 0 = 0
    total = total + va(nil, 1)        -- 0 + 1 = 1

    -- 6. 原生函数：多传丢弃
    total = total + math.floor(1.9, 99)             -- 1
    total = total + #string.rep("ab", 2, "-", "x")  -- 5

    return total
end
