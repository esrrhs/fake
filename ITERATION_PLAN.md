# fakelua 下一轮迭代清单

> 基于 master @ `a55a4bf`（已与 origin/master 同步）的代码审计与实测。
> 每项都标注了代码位置、验证方式和建议执行顺序。

---

## P0″ · 本轮 bug 排查修复（2026-10-14，未提交）

以真实 Lua 5.4.9 作 oracle 做三后端差分测试，修复 4 组语义 bug。
差分脚本与用例在 `/tmp/flaudiff/`（`cmp_local.py` + `cases*.lua`），
oracle 是本地编译的 `/tmp/lua-5.4.9/src/lua`。

### Bug 1 · 原生函数当值使用（编译期报错缺失）

`local t = type; t(123)`、`pcall(table.concat, t)`、`{cb = string.format}` 在
运行时才报误导性错误（"attempt to call a non-function value"）。
原生函数由编译期按名派发，不是一等值。

**修复**：`semantic_analysis.cpp` 的 `check_simple_var` 与 `case Var:` 的 kDot 链
检查里，命中 `FindNativeFunction` 时直接给编译期明确报错；`math.pi`、
`string.charpattern` 等常量不受影响。回归测试见 `test_syntax_tree.cpp` 的
`semantic_native_function_as_value_*`。

### Bug 2 · 可选参数显式 nil 被拒

`table.concat(t, "-", nil, nil)` 报 "bad argument #3 (number expected)"，
而 5.4 的 `tconcat` 用 `luaL_optinteger`，显式 nil 与省略等价。
逐函数对照 5.4.9 源码修正 10 处（`checkinteger` 的保持报错）：
`table.concat/unpack`、`string.byte/sub-j/rep-sep/find-init/match-init`、
`utf8.codepoint/len/offset-i`。回归测试：`test_*_optnil.lua` 三份。

### Bug 3 · 函数调用实参个数语义（三道 arity 门，全部与 Lua 不符）

Lua 允许实参个数与形参不一致：少传的形参为 nil，多传的实参求值后丢弃。
fakelua 有三道门分别拒绝，动态调用路径（`FlCallClosure`）却已是 Lua 语义 —— 同一个
调用经变量转发能过、直接调用报错，语言内部不自洽。

| 门 | 位置 | 现象 | 修复 |
|---|---|---|---|
| 编译期直接调用检查 | `c_gen.cpp:CompileFunctioncall` | `f(1)` 报 "wrong number of arguments" | 删除检查 |
| 数学特化直调 | `c_gen.cpp:TryCompileSpecDirectCall` | `add(1,2,3)` 泄漏 TCC 原始错误 `<string>:N: error: too many arguments to function` | 参数个数不符时回退普通路径 |
| 按名分发运行时检查 | `vm.cpp:CallByNameImpl` | `FakeluaCallByName` 报 "expects N argument(s)" | 删除检查（紧跟其后的循环本就补 nil/截断） |

配合修改 `BuildLocalFunctionCall` 非 vararg 分支：少传补 `kNil`、多传截断，
展开位置超出形参个数时丢弃展开值；`FlUnboxMulti` 越界本来就是 nil。

**契约变化**：`FakeluaCallByName` 不再对参数个数不符抛异常（Bug M 担心的读未初始化
栈内存，由派发前填满 `temp_arg_arr` 保证）。`test_runtime.cpp` 的
`vm_call_by_name_error_cases` 与 `vm_cvar_call.multi_return_expansion` 已相应改为
断言补齐/截断语义（含"补的是 nil"的断言）。

三个原断言"少传参数=编译错误"的异常测试（`spec_call_arg_count_error`、
`math_spec_too_few_args`、`no_arg_call`）已改为 Lua 语义：编译通过 +
`Call` 时抛 "attempt to perform arithmetic on non-numeric value"。

### Bug 4 · interp 后端 CALLNAME 误判文件级局部变量

`local gv = f; gv(1)` 在 interp 下报 `FakeluaCallByName: function 'gv' not found`。
`codegen.cpp` 的 `file_level_func` 只看 `defining_func == nullptr`，把文件级
local **变量** 也当成文件级 local **函数**，而 `named_protos_` 只登记函数定义。
改为同时要求 `def_node->Type() == LocalFunction`。C 后端此处按闭包处理，本就正确。

### 连带修复 · 原生函数必填参数缺失静默通过

上面 Bug 3 移除运行时检查后，缺参调用真正走进原生函数了，暴露出既有写法
`if (n < 1) return nil;` 把「缺失」与「显式 nil」混为一谈（Lua 下 `type()`、
`string.byte()`、`table.concat()` 都报错）。已按 5.4.9 把 25 处必填参数的守卫
改为 `ThrowBadArgument`：`type/tostring/tonumber`、`string.{len,rep,reverse,
lower,upper,byte,format,find,match,gmatch,gsub,dump,pack,packsize,unpack,load}`、
`table.{insert,remove,concat,unpack,move,sort}`、`math.{max,min}`、
`utf8.{codepoint,codes,len,offset}`。`table.create`/`loadfile`/`dofile` 的
缺参默认值是 fakelua 自有语义，保持不动。

### 验证

- 新增 `test/lua/basic/test_basic_call_arity.lua` + `test_basic.call_arity`
  （期望 92，三后端 × 直接调用/闭包变量/按名分发/多值展开/vararg/原生函数）。
- 差分 `cases9_arity.lua` 35 例、`cases10_arity2.lua` 12 例、
  `cases11_missing_args.lua` 35 例：三后端 0 分歧；旧 `cases7` 41 例、
  `cases8` 23 例回归无变化（cases7 的 2 例仅报错文案前缀差异）。

### 已发现未修（下轮候选）

1. **零值返回值在原生快路径被塌缩为 nil**：`tostring(select(2, 9))` 在 Lua 下
   是把 0 个值传给 `tostring` → 报 "bad argument #1 to 'tostring' (value expected)"；
   fakelua 的 `tostring`/`tonumber` 快路径用 `CompileExp(arg)` 不保留 multi，
   零值 multi 变成 nil → 返回 "nil"。修法是快路径保留 multi 并在
   `type_ == VAR_MULTI` 时回退 `FakeluaCallByName`（由它摊平）。
   `#{pick(...)}` 这类表构造路径已正确（0）。
2. `function obj:m() ... end` 方法定义在 fakelua 不支持
   （`SemanticAnalysis: Unsupported function name with method definition`）；
   文件级 `local t = {}` 是常量表，不能改字段。

---

## P0′ · 数学特化函数非数值参数去优化 ✅ 已完成（2026-10-11）

### 问题（master `9fd9be6` 实测）

文件级数学特化函数收到**非数值 math 参数**（string/bool/nil/table，或少传参
补的 nil）时，TCC/GCC 的入口分发器直接 `FakeluaThrowError`。该异常逃逸生成
代码后无人捕获 → `terminate due to uncaught exception`，**整个进程 abort**；
INTERP 后端却正常执行函数体。三后端不一致，且 Lua 层 `pcall` 无法拦截。

```lua
function f(x)
    if type(x) ~= "number" then return -1 end  -- 合法的 type 守卫
    return x + 1
end
f("abc")  -- interp: -1；TCC/GCC(master): 进程崩溃
```

### 修复

- 每个特化函数额外生成一份 `__dyn` 全动态变体（`CompileFuncBody(..., -1)`，
  与非特化函数同一路径），入口分发器检测到任一 math 参数非 int/float 时
  分派到它，不再抛错；函数体内真正的算术错误仍走可 pcall 的 Lua 错误路径。
- 新增 `FakeluaSpecMissException`（`exception.h`）：试探性原生数值编译失败
  （`CompileNumericExp` 等 7 处 `ThrowError` 改用 `ThrowSpecMiss`）不再刷
  ERROR 日志——这是「试不了原生就回退动态」的常规控制流；逃逸出 `Build`
  顶层时仍转换为与历史文本一致的硬编译错误。
- `GenerateDecls` 补 `__dyn` 前向声明。

### 验证

- 新增 `test/lua/infer/test_spec_deopt.lua` + `infer.test_spec_deopt`
  （三后端 × debug 开关；含 type 守卫、多参数、异类返回、pcall、递归、
  C++ 边界 catch）。
- 差分 `.workbuddy/difftest/cases6_deopt.lua`：32 例 vs Lua 5.4.9，0 分歧；
  旧 5 集 223 例回归，21 分歧全为已知范围外项（字符串比较刻意不做、
  find/next nil 尾参 bug1 暂缓、%q/%c 版本差异）。
- 形态断言污染（第二轮回归发现，共 14 个）：`__dyn` 全动态体天然含
  IsTrue/CVar/flua_rbt_ 等，使「特化变体中不应出现 X」的负向断言失效。
  修复方式：测试 helper `StripDynVariantBodies` 下沉进 `InferGetCCode`，
  **默认剥离 `__dyn` 定义体**（前向声明与入口分发器保留），仅
  `test_spec_deopt` 以 `keep_dyn_variants=true` 取原始代码；既有 100+ 处
  形态断言零改动。
  - 教训：全量回归若经 `tail` 管道会截断 gtest 失败汇总（曾看到 37 失败
    却只显示 5 个），**必须重定向到文件再 grep**。
- infer 套件 220/220 全过（TCC/GCC/interp × debug on/off）；
  全量 1500+ 测试：除 mysql integration（需真实数据库，环境基线）外全过。

---

## P0 · 修复 macOS 上测试套件的可用性 ⚠️ 实现在 PR #25，**已被用户关闭，勿重做**

> 状态澄清（2026-10-11 核实）：以下方案曾完整实现于远端分支
> `fix/jit-compile-cache-and-reproducible-codegen`（PR #25，`76caa37`），
> 但 PR 被**用户故意关闭**，代码从未合入 master（master 至今仍硬编码
> `static void * _S = (void*)<addr>`，无 gcc_cache）。本地全量 GCC 测试
> 每个用例冷 dlopen ~4.5–5.4s 是常态，全量约 40 分钟。**不要重新提交该 PR。**



### 问题

在 macOS 上单个 gtest 用例耗时 **30.5 秒**（本应是毫秒级），全量测试实际无法在本地跑完。

### 根因（已隔离复现，非推测）

`GccJitter::Compile` 的流程是 `fork()` → `execvp("gcc")` → `waitpid()` → `dlopen()`，
每次编译生成一个**全新路径的临时 dylib**。实测数据：

| 场景 | 耗时 |
|---|---|
| 裸 `gcc -O3 -dynamiclib` | 0.06 s |
| 首次 `dlopen` 某新路径 dylib（4 个路径实测） | **4521 / 4504 / 5437 / 4608 ms** |
| 后续 `dlopen` 同一路径 | 0.2 ms |

**每一个新 dylib 路径的首次加载都要付约 4.5–5.4 秒**，与文件内容无关，
纯粹是 macOS 代码签名验证（amfid / syspolicyd IPC）的固定开销。差值达 25000 倍。

排查中排除的两个假设（避免后来者重走）：
- **ad-hoc 签名无效**：实测 `codesign -s -` 为 6312/5106/5222 ms，`-Wl,-adhoc_codesign`
  为 4852/4753/4672 ms，**都不比基线好**。这条路不通。
- **fork+dlopen 组合本身有问题**：写隔离复现程序跑 15 轮全部秒过，假设不成立。

真正起作用的验证是「同路径 vs 新路径」的对照测量。

### 顺带发现的两个真实 bug

1. **生成的 C 代码硬编码了 State 指针**（`c_gen.cpp:451`）：
   ```c
   static void * _S = (void *) 0x1014b2aa0;   // ← 地址受 ASLR 影响，每次不同
   ```
   这导致同一输入编译出不同字节 —— 构建不可复现，且**产物一旦被跨 State 复用，
   `_S` 会指向别的 State**。已改为初值 0 + 加载后注入（见下）。

2. **`FAKELUA_BUILD_TESTS` 默认为 OFF**，导致 `ctest` 找不到任何测试
   （`ctest --test-dir build` 输出 `No tests were found!!!`）。本地 build 目录里
   `unit_tests` 二进制可能停留在几天前的版本，造成"改了代码却看不到效果"的假象。
   **这条坑了我很久**：连续多轮"验证失败"其实都在跑 3 天前的旧二进制。

### 最终实现

- **State 指针改为运行时注入**：`c_gen.cpp` 生成
  `static void * _S = 0;` + `void __fakelua_set_state(void*)`，
  GCC 与 TCC 两条路径在加载产物后、调用 `__fakelua_init` 之前注入。
  （注意 TCC 以 C89 模式编译，**不能用 `nullptr`**。）
- **按内容缓存编译产物**：`src/jit/gcc_cache.{h,cpp}` 以
  「生成的 C 代码 + 全部编译参数」的 xxhash-64 摘要为键，命中即复用同一路径，
  从而命中 dyld 的路径缓存。写入用「临时文件 + rename」保证原子性。
- **可关闭**：`StateGCCConfig::enable_compile_cache = false` 恢复旧行为。

实测效果（`closure` 套件，11 个测试）：

| | 耗时 |
|---|---|
| 修复前 | 487 s |
| 冷缓存首次 | 96 s（`_S` 修复带来的编译去重已生效） |
| 热缓存 | **13 s** |

单个用例：35 s → **0.106 s**。
`test_basic/test_math/test_string/test_table/syntax_tree/vm_cvar_call/runtime/state/var/util`
共 387 个测试热缓存下 106 s，全部通过。

### 遗留

- 缓存目录 `$TMPDIR/fakelua-jit-cache` **不会自动清理**，需要手动删除。
  刻意不放在 `$TMPDIR/fakelua/` 下 —— 那个目录会被
  `test_runtime.cpp` 的 `runtime.generate_tmp_filename_creates_dir` 用 `remove_all` 清掉。
- CI 三个 workflow 仍无超时保护，建议补 `--gtest_timeout` 或外部 timeout。

---

## P1 · VarTable 增加数组部分（性能收益最大）

### 问题

`src/var/var_table.h:11-43` 的 `VarTable` 只有 `quick_data_[8]` + 哈希桶，
**没有 Lua 那样的连续数组部分**。因此整数键读写每次都要：
类型检查 → `spec_get` 间接调用判断 → `count_ == 0` 检查 → 算 hash → 判断 `bucket_count_` → **遍历桶链比对**（`c_runtime_header.h:944-976`）。

### 为什么这是主要瓶颈

实测 Sieve 生成的 C 代码里，**算术已经 100% 原生化**
（`FL_INT_MUL` / `FL_INT_ADD` / int64_t 循环变量），GCC 版 Sieve/5000 = 213μs
甚至比 C++ 参考实现快 2.68 倍。**唯一剩余瓶颈就是表访问。**
Lua 5.4 正是靠数组部分赢的。

「数值特化」这条路已经走到尽头，下一轮收益在表访问而非算术。

### 方案

给 `VarTable` 增加 `CVar* arr_; uint32_t arr_len_;`：

- `FlSetTableIntImpl` 检测 `k == arr_len_ + 1` 时 append
- `FlGetTableInt` 命中 `1 <= k <= arr_len_` 时直接 `arr_[k-1]`
- 删除时保守退回哈希（只增不删最简单）

### 代价与风险

`var_table.h:8` 明确注释「layout must match exactly」——
`sizeof(VarTable)` 变化必须同步 `c_runtime_header.h` 里的镜像定义。
**布局不同步会导致难以排查的内存问题。**

### 预期收益

Sieve / BubbleSort / BinarySearch / Vector3 的**共同**瓶颈，
`for i=2,n do is_prime[i]=true end` 是教科书式顺序填充，能立刻吃满。预计 **2–4x**。

---

## P2 · spec table 字段按推断类型降为原生标量

### 问题 —— 「已经算出来却丢弃」的信息

`type_inferencer` 确实算出了 spec 字段的类型，但随后被显式丢弃：

- `type_inferencer.cpp:1771` — `f.type = T_DYNAMIC; // 类型由 CGen 在 emit 时重新推导`
- `c_gen.cpp:411` — `EmitSpecTypeBoilerplate` 无条件写 `CVar`
- `c_gen.cpp:838` — `GetSpecFieldType()` **全仓库唯一定义、零调用**（死代码，坐实了类型被丢弃）

### 实测后果

```c
// local p = {x=i, y=i*2, z=i*3}; total = total + p.x + p.y + p.z
CVar total = (CVar){.type_ = VAR_INT, .data_.i = 0};      // 本该是 int64_t
for (...) {
    OpAdd((total), (FL_SPEC(spec_x, p, _s_x)), flua_op_20); // 每次重新 dispatch
    ...
}
```

对比**同一文件**中未被 spec 化的 `s = s + i`：`s = FL_INT_ADD((s), (i));` —— 纯原生、无装箱。
`OpAdd`（`c_runtime_header.h:1154-1158`）每次执行 2 次 `CheckNum` + 1 个分支 + 一次 16 字节 CVar 写回。

### 方案

1. `BuildCtorFields` 改为读 `main_eval_types` 的真实推断类型，替换 1771 行的硬编码
2. `EmitSpecTypeBoilerplate` 按 `f.type` 发 `int64_t` / `double` / `CVar`
3. `FL_SPEC` 读取点据 `GetSpecFieldType` 决定返回原生表达式还是 CVar

### 关键难点

**双写一致性**：`FL_SET_SPEC`（`c_runtime_header.h:686-691`）与 spec_set 回调
（`c_gen.cpp:287`）必须做「原生字段 ↔ spec_vals[]」双写，
否则 `pairs` / 动态 key 读到的会是脏值。这是本项唯一的技术风险，
必须配合 optional 字段的 nil 语义仔细处理。

### 预期收益

table 密集的数值场景预计 **1.3–1.8x**。因为是「已算出来却丢弃」的信息，ROI 最高。

---

## P3 · interp 与 GCC 后端的行为一致性

`src/interp/` 与 `JIT_GCC` 的**语言特性覆盖基本等价**：
32 个 AST 节点、47 个 opcode、20 个二元 + 4 个一元运算符全部对齐；
标准库**完全共享** `src/native/`，无后端差异。
真正的 interp 独有缺口只有 4 个，严重度都不高，但都会导致
「只在 interp 下跑通的脚本在 GCC 下失败」这类问题：

| # | 问题 | 位置 | 严重度 | 状态（2026-10-11 复测） |
|---|---|---|---|---|
| 1 | `local` 与全局常量名冲突未检查 —— GCC 编译报错，interp 静默通过 | 已移入共享阶段 `semantic_analysis.cpp:1047`（`CheckLocalVar`） | 中 | ✅ 已修复，三后端同文报错 |
| 2 | `GETTABLE` 读路径漏 NaN 检查 —— `t[0/0]` 读不报错但写报错，读写不对称 | 三后端现已收敛一致 | 中低 | ⚠️ 后端间已一致（都报 `table index is NaN`，pcall 可拦），但**读路径比 Lua 5.4 严格**：Lua 读 `t[NaN]` 合法返回 nil，仅写报错；若要严格对齐需放开 GETTABLE 的 NaN 读 |
| 3 | 寄存器宽度硬上限 256（CGen 无此限制） | `codegen.cpp:34-36`、`opcode.h:66-69` | 中低 | 未复测 |
| 4 | 深递归栈溢出错误类型不一致（init 内干净报错 / init 外变 `bad_alloc`） | `interpreter.cpp:75-80` | 低 | 未复测 |

### 测试盲区

- **全部 46 个测试文件中 `JIT_INTERP` 出现次数为 0** —— 覆盖全靠 `AllJitTypes()` 间接循环
- `test_state.cpp` / `test_runtime.cpp` 把后端写死（TCC 15 次 / GCC 7 次），**排除了 interp**，
  而 interp 恰是唯一依赖 `IsInterpClosure` 魔数判定的后端
- `test/lua/` 下 **978 个 `.lua` 语料中 724 个从未被任何测试驱动**
  （`test_jitter.cpp` 只引用 254 个），这些文件在任何后端下都没被执行过

### 建议

为缺口 1 和 2 补**定向断言**（明确断言 interp「应该」和 GCC「应该」的行为一致），
并把 `test_state.cpp` / `test_runtime.cpp` 改用 `AllJitTypes()`。

---

## 明确不要做的方向

| 方向 | 原因 |
|---|---|
| **优化 TCC 后端** | 实测 TCC 在 BubbleSort/Sieve/InsertionSort 上**快于 Lua 5.4 达 2.3–2.8x**，只是比 GCC 慢 4–7x。同一份 C 代码 GCC 快 3–30x —— 这是 TinyCC 优化器能力不足（无向量化/跨函数内联），不是 c_gen 的问题 |
| **给 `FL_INT_*` 加溢出检查** | `c_runtime_header.h:1144-1146` 用无符号算术**故意**实现回绕，这正是 Lua 5.4 整数语义。加检查会同时破坏性能和语义 |
| **给 string/bool 加类型维度** | 是刻意的设计边界而非遗漏。string 变长 + 引用计数 + 共享，原生化引入所有权问题；bool 无法用 C `bool` 表达（Lua 中 `0` 和 `""` 都是真值） |
| **为 `and`/`or` 生成 C 的 `\|\|` / `?:`** | 语义不等价 —— Lua 中 `0` 和 `""` 都是真值。必须走完整三值判定 |
| **字符串关系运算符（`<`/`>`/`<=`/`>=`）** | **刻意不做（用户 2026-10-06 拍板）**，非遗漏。fakelua 是静态类型子集，为性能不做任何运行时隐式转换，`"a" < "b"` 报错与 `"10" + 1` 报错是同一条边界。字符串 `==`/`~=` 仍支持（不涉及转换）。差分测试曾把它记为 bug 2，结论作废 |

---

## 其他可清理项

- **README 文档过时**：`README.zh.md:114` 仍在展示 `NativeAdd(...)`，
  但全仓库 grep 无此符号 —— 已被 `FL_INT_ADD` 宏（`c_runtime_header.h:1144`）取代。
- **残留分支**：`fix/open-bugs`（`e2e730b`）内容与 master 的 `a55a4bf` 相同，可删除。
- **低风险优化**：`c_gen.cpp:4592/4602/4611` 生成的 `SET_NIL(flua_call_N)` 是死存储
  （`t[i]=v` 被预处理重写成函数调用，必然 `return tmp` 一个永不被读取的 nil）。
  Sieve 每轮循环一次。消除它零风险且能改善寄存器压力。
