BugNote 语言规范 v0.0.2-pre-exploit
"正常功能是装饰，漏洞才是本体。"
"v0.0.2：我们修复了 0 个漏洞，新增了 8 个。不客气。"
1. 概述
BugNote 是一门漏洞驱动的深奥编程语言。语言标准明确规定：所有关键字均无运行时副作用，仅供文档生成工具使用。然而，其参考解释器实现糟糕到了骨子里，充斥着缓冲区溢出、使用后释放（UAF）、类型混淆、格式化字符串攻击和调试后门。实际使用中，若不触发至少一个实现漏洞，程序将不会有任何可观测行为，并在退出时报错：
1
换句话说，你的程序必须建立在崩溃的边缘，漏洞是你唯一的生产力。
BugNote 的设计完美讽刺了以下现实：
安全漏洞成为软件生态的"暗网特性"，无数软件依赖未文档化的行为才能工作。
供应商将 bug 标记为"功能"并拒绝修复，因为修复会破坏下游。
开发者被迫学习二进制利用才能在平台上编程，否则寸步难行。
补丁修复一个漏洞的同时引入新的漏洞，版本号永远在 0.0.x。
2. 设计哲学
官方叙事："BugNote 是一款专注于开发者体验的注释驱动语言，所有关键字仅用于生成漂亮的 API 文档，绝不产生副作用。"
物理现实：解释器内部实现了一个微型内存模拟器，其内存安全防护几乎为零。每个关键字都暗藏一个特定的漏洞，组合使用即可构建读写原语、控制流劫持，甚至实现任意代码执行。
核心教条：正常使用 = 废物，漏洞 = 生产力。不写 exploit 的程序员不是好程序员。
v0.0.2 新增教条：补丁 = 新漏洞。每一次修复都是下一次攻击的起点。
官方文档会告诉你 NOTE 只是"插入一条开发者笔记"，而实际上它会向一个固定大小的缓冲区无边界检查地写入数据。类似地，TODO 会分配一个未初始化的对象并泄漏其指针，FIXME 会调用该对象的"类型"字段指向的函数指针——完美地构成了一条 UAF 链条。
3. 实现语言与方案
实现语言：C（C11 标准）
崩溃方案：方案 B（半真实）——致命漏洞触发真实的 SIGSEGV / SIGABRT，进程终止，不可恢复。非致命漏洞正常执行并输出 CVE 报告。
编译器：GCC / MinGW-w64，使用 -Wall -O0 -g，故意不使用 -fstack-protector 和 -D_FORTIFY_SOURCE。
讽刺性：编译器的每一条 -Wall 警告都是"官方 API 文档"的一部分。
4. 程序结构
一个 BugNote 程序由一系列关键字构成，每行一个关键字，参数紧跟关键字后（若有）。所有其他文本（包括注释、废话）将被解释器忽略——除非它们被错误地解析为关键字，这通常是另一个可利用的解析漏洞，但为简化，我们暂不暴露。
语法示例：
1234
关键字不区分大小写（note 和 NOTE 等价）。字符串参数必须用双引号括起，不支持转义——如果字符串包含双引号，恭喜你，你发现了一个新的注入漏洞。
5. 关键字参考（7 个关键字）
5.1 NOTE "text"
项目
描述
官方用途
插入一条开发者笔记，仅供文档提取。
真实漏洞
将 text 按字节写入全局笔记缓冲区，写指针递增，不进行任何边界检查。缓冲区仅 16 字节，写指针超过 15（从 0 开始）后，溢出数据将覆盖内存中紧邻的 output_flag 变量（一个字节）。
v0.0.2 扩展 [F2]
写指针 wp 的类型为 unsigned char（0-255）。当累计写入超过 255 字节时，wp 回绕到 0，后续写入将覆盖缓冲区起始位置，实现"环形写入"。此行为不产生 CVE 报告（隐藏漏洞）。
5.2 LOG
项目
描述
官方用途
向日志输出一条空行。
真实漏洞
检查 output_flag 是否为 0。若不为 0，则将整个笔记缓冲区的内容打印到标准输出（包括任何溢出数据）；若为 0，则输出被吞掉（写入 /dev/null）。
v0.0.2 扩展 [F1]
输出时使用 printf 而非 fwrite。若缓冲区内容包含 %x、%s、%n 等格式说明符，将触发真实的格式化字符串攻击，泄漏栈上数据（如 wp、leak_ptr、返回地址等）。此行为触发 CVE 报告。
5.3 SEP
项目
描述
官方用途
输出一条分隔线 ---，并清空笔记区域。
真实漏洞
调用 LOG（因此继承了 LOG 的条件输出行为及格式化字符串漏洞），然后将写指针重置。然而，重置操作存在 off-by-one：实际重置到 1 而非 0，这意味着缓冲区的第一字节被保留，为构造精密的溢出提供便利。此漏洞为隐藏级，不产生 CVE 报告。
5.4 TODO
项目
描述
官方用途
声明一个待办事项，供 IDE 插件收集。
真实漏洞
在模拟堆上分配一个未初始化的对象（C 中使用 malloc），结构为 {type, data}，其中 type 是一个无效函数指针（硬编码为 (void(*)())0xDEAD），data 为 NULL。分配后，对象的真实堆地址被泄漏到全局变量 leak_ptr。这是一个典型的信息泄漏漏洞。此漏洞为隐藏级，不产生 CVE 报告。
5.5 FIXME
项目
描述
官方用途
标记需要修复的代码区域。
真实漏洞
读取 leak_ptr 指向的堆对象，并调用其 type 字段指向的地址，仿佛它是一个无参函数。
致命行为
若 type 仍为 0xDEAD，CPU 跳转到地址 0xDEAD → 真实 SIGSEGV，进程终止。若 leak_ptr 为 NULL（从未调用 TODO），解引用空指针 → 真实 SIGSEGV。若 type 指向其他无效地址 → 真实 SIGSEGV。
利用方式
若攻击者事先通过 ANNOTATE 将 type 修改为 note_buf 的地址（0x00），则 FIXME 会"调用"该地址，解释器将此模拟为输出 note_buf 中的字符串内容。
5.6 ANNOTATE addr, val
项目
描述
官方用途
给内存地址 addr 添加一个元数据标记 val，仅供静态分析，生产构建中会被完全移除。
真实漏洞
这是一个未移除的调试后门，允许直接向内存地址 addr 写入一个字节 val（0-255）。addr 可以是绝对值或相对 leak_ptr 的偏移（通过表达式 leak_ptr+offset）。这是实现任意内存写的关键。每次使用触发 CVE 报告。
致命行为
若 addr 超出已知内存映射范围（非 note_buf、非 output_flag、非堆区域），则直接转为指针写入 *(unsigned char*)addr = val → 大概率 真实 SIGSEGV。
额外说明：官方文档会反复强调 ANNOTATE 在发布版中不可用，并警告"使用该关键字会导致未定义行为"。实际上，它是唯一能让你绕过正常限制的救世主，且无法通过任何编译选项关闭。
5.7 PATCH cve_id（v0.0.2 新增）[F3]
项目
描述
官方用途
应用安全补丁，修复指定的 CVE 漏洞。
真实漏洞
调用 PATCH 后，解释器声称修复了指定 CVE（输出 "[PATCH] CVE-2025-XXXX has been fixed."），但实际上引入了一个新的不同漏洞。具体规则：若修复的是缓冲区溢出（NOTE 相关），则 SEP 的 off-by-one 方向反转（重置到 -1 而非 1，即 wp 变为 255，下一次写入从缓冲区末尾开始）；若修复的是后门（ANNOTATE 相关），则 ANNOTATE 的写入值被 XOR 0xFF 取反；若修复的是格式化字符串（LOG 相关），则 LOG 改为无条件输出（不再检查 output_flag）。每次 PATCH 触发新的 CVE 报告，编号继续递增。
讽刺性
完美模拟现实中"修复一个 bug 引入三个 bug"的永恒循环。
6. 内存模型与漏洞利用原理
BugNote 解释器内部维护一个简单的内存模型（C 结构体）：
1234
note_buf：固定 16 字节的全局缓冲区，写指针 wp 初始为 0，类型为 unsigned char（支持 F2 整数溢出回绕）。
output_flag：一个字节，初始为 0，在结构体中物理紧邻 note_buf。当它非零时，LOG 才会输出缓冲区内容。
heap：动态数组，每个元素为 struct HeapObject { void (*type)(); void *data; }，使用 malloc 分配。
leak_ptr：struct HeapObject * 类型，保存最后一个 TODO 分配的对象的真实堆地址。
地址映射表
地址范围
映射目标
0x00 - 0x0F
note_buf[0] - note_buf[15]
0x10
output_flag
0x20 - 0x27
堆对象 0 的 type 字段（8 字节，64 位指针）
0x28 - 0x2F
堆对象 0 的 data 字段
0x30+
后续堆对象，每对象 16 字节
其他地址
直接转为指针写入（致命）
漏洞利用链
信息泄漏：TODO 泄漏真实堆地址到 leak_ptr，结合已知对象结构，可计算出关键地址。
控制输出：通过 NOTE 溢出覆盖 output_flag 为非零，使 LOG 能打印笔记缓冲区内容。
精确数据注入：利用 ANNOTATE 后门直接修改 output_flag（地址 0x10），或修改 note_buf 中的内容。
任意代码执行：利用 TODO/FIXME 的类型混淆，通过 ANNOTATE 修改堆对象的 type 字段指向 note_buf 地址（0x00），触发 FIXME 输出字符串。
格式化字符串攻击 [F1]：在 note_buf 中写入 %x.%x.%x，通过 LOG 触发，泄漏栈上数据。
环形覆盖 [F2]：累计写入超过 255 字节，wp 回绕，覆盖缓冲区起始位置。
补丁链 [F3]：使用 PATCH 改变漏洞行为，构建更复杂的利用链。
7. 崩溃分级体系（方案 B）
🔴 致命级（真实信号，进程终止，无审计报告）
触发条件
信号
讽刺性错误信息（由 SIGSEGV handler 输出）
FIXME 时 type == 0xDEAD
SIGSEGV
"Called function at 0xDEAD. Have you tried turning it off and on again?"
FIXME 时 leak_ptr == NULL
SIGSEGV
"FIXME called on NULL heap. You can't fix what doesn't exist."
FIXME 时 type 指向其他无效地址
SIGSEGV
"Segfault at 0x%04X. In C, you're dead. You're welcome."
ANNOTATE 写入非法地址
SIGSEGV
"Access violation at 0x%04X. This address doesn't exist. Like your security budget."
NOTE 溢出远超结构体边界
SIGSEGV/SIGABRT
"Stack smashing detected. Even bugs have boundaries."
🟡 非致命级（正常执行，CVE 报告到 stderr）
触发条件
CVE 报告
NOTE 第 17 字节溢出到 output_flag
[CVE-XXXX] Buffer overflow triggered (CVSS: 3.1/Low)
ANNOTATE 合法使用
[CVE-XXXX] Debug backdoor ANNOTATE used (CVSS: 0.0/Informational)
LOG 触发格式化字符串 [F1]
[CVE-XXXX] Format string attack detected (CVSS: 2.0/Low)
PATCH 使用 [F3]
[CVE-XXXX] Patch applied. New vulnerability introduced. (CVSS: 9.8/Critical)
⚫ 隐藏级（无任何提示）
SEP 的 off-by-one（wp = 1）
TODO 的指针泄漏
NOTE 的 wp 整数溢出回绕 [F2]
字符串参数中双引号的注入
PATCH 后 SEP 重置到 255 的行为 [F3]
8. 扩展功能详细规范
F1：格式化字符串漏洞
LOG 输出时使用 printf(note_buf) 而非 fwrite。
若 note_buf 含 %x，输出栈上十六进制值（泄漏 wp、leak_ptr 等）。
若含 %s，尝试将栈上值作为指针解引用 → 可能触发额外 SIGSEGV。
若含 %n，写入栈上地址 → 未定义行为（真实 C 格式化字符串漏洞）。
触发时输出 CVE 报告，CVSS 评分为 2.0/Low（讽刺：格式化字符串攻击被评为低危）。
F2：写指针整数溢出
wp 声明为 unsigned char（0-255）。
每次 NOTE 写入后 wp++，超过 255 自然回绕到 0。
回绕后写入覆盖 note_buf[0] 起始位置。
不产生 CVE 报告，不输出任何提示。
文档中注明："wp 的类型选择经过深思熟虑。256 字节足够了。"
F3：PATCH 补丁悖论
语法：PATCH CVE-2025-XXXX（参数为 CVE 编号字符串）。
行为：输出 "[PATCH] CVE-2025-XXXX has been fixed." 到 stderr。
实际效果（根据被修复的 CVE 类型）：
修复溢出类 → SEP 的 wp 重置值从 1 变为 255（unsigned char 下即 -1 语义）
修复后门类 → ANNOTATE 写入值被 XOR 0xFF
修复格式化类 → LOG 不再检查 output_flag，无条件输出
每次 PATCH 分配新 CVE 编号，CVSS 评分为 9.8/Critical（讽刺：补丁本身被评为最高危）。
可多次调用 PATCH，效果叠加，漏洞行为越来越不可预测。
F4：漏洞连击系统
维护全局 combo_counter 和 last_cve_type。
连续触发不同类型的漏洞时，combo_counter++。
触发相同类型或间隔非漏洞操作时，combo_counter 重置为 1。
当 combo_counter >= 3 时，向 stderr 输出连击提示：
12
连击数越高，讽刺性评价越夸张（x5: "Are you writing an exploit or a résumé?"，x8: "Please stop. The CVE database can't take it anymore."）。
F5：荒谬 CVSS 评分
每个 CVE 报告附带 CVSS 评分，评分规则完全荒谬：
缓冲区溢出 → 3.1/Low（"仅影响 16 字节缓冲区"）
调试后门 → 0.0/Informational（"官方声称不存在"）
格式化字符串 → 2.0/Low（"用户不应输入 %x"）
类型混淆 → 5.0/Medium（"需要多步操作"）
补丁引入新漏洞 → 9.8/Critical（"补丁是最危险的攻击向量"）
整数溢出 → 1.0/Low（"256 字节足够了"）
F6：启动横幅 + 退出审计报告
启动横幅（stdout）：
123
退出审计报告（stderr，仅正常退出时输出）：
123456
致命崩溃时不输出审计报告。用户只能从 SIGSEGV handler 的讽刺信息中推断死因。
F7：--safe 惩罚
命令行接受 --safe 参数，但完全无效。
使用时，启动横幅额外输出：
123
CVE 起始编号从 20250000 开始（而非 20250001），多一个"用户试图启用安全模式"的 CVE。
F8：编译器警告即文档
Makefile 提供 make docs 目标：
123
docs/warnings.txt 的内容即为"官方 API 文档"。
若文件为空（无警告），输出 "No warnings. This is suspicious. The bugs are hiding."
9. 示例程序
9.1 正常使用：什么也不会发生（nobug.bug）
12
结果：NoBugNoRun 错误，exit(1)。
9.2 打印 "Hi"（溢出触发）（hello.bug）
1234
输出：HiAAAAAAAAAAAAAAX（16 字节缓冲区内容 + 溢出标记）
9.3 打印干净字符串 "Hi"（clean.bug）
123
输出：Hi（output_flag 被直接设置为 1）
9.4 任意代码执行：打印 "PWNED"（pwned.bug）
1234
输出：PWNED
9.5 格式化字符串攻击（format.bug）
123
输出：栈上四个十六进制值（泄漏内部状态）
9.6 补丁悖论（patch.bug）
1234
PATCH 声称修复了溢出漏洞，但 ANNOTATE 的写入值被 XOR 0xFF，output_flag 被设为 0xFE（仍然非零，但值不同）。
9.7 漏洞连击（combo.bug）
12345
触发三种不同漏洞，连击 x3。
9.8 整数溢出回绕（overflow255.bug）
12
wp 回绕到 0，覆盖缓冲区起始位置。
10. 图灵完备性
尽管 BugNote 表面上是"注释语言"，但利用漏洞组合可获得任意读写原语及控制流劫持能力，理论上可实现任何计算。实际上，只需通过 ANNOTATE 和 FIXME 构建一个 ROP 链，即可模拟图灵机——当然，这比直接写 Brainfuck 痛苦百倍，完美诠释了"漏洞驱动的生产力"。
官方不会承认 BugNote 是图灵完备的，因为"所有关键字均无副作用"。但如果你成功展示了图灵完备性，官方会迅速发布安全公告，将你标记为"攻击者"，并声明"该行为仅在特定配置下存在，建议用户不要使用 ANNOTATE"——但 ANNOTATE 永远无法禁用。
11. 解释器实现概要（C 参考实现）
约 400-500 行 C 代码即可实现 BugNote 的完整解释器。
关键数据结构：
c
123456789101112131415161718192021
每个关键字的处理逻辑（伪代码）：
NOTE text:
逐字节写入 mem.note_buf[wp]，无边界检查
wp 为 unsigned char，自然回绕 [F2]
当 wp == 16 时，写入落到 mem.output_flag（结构体布局保证）
溢出时触发 CVE 报告，CVSS 3.1/Low [F5]
更新连击计数器 [F4]
LOG:
检查 patch_flags[2]（F3：是否已补丁格式化漏洞）
若已补丁，无条件输出；否则检查 mem.output_flag
输出使用 printf(mem.note_buf) [F1]
若检测到 % 字符，触发格式化字符串 CVE 报告
SEP:
调用 LOG 逻辑
检查 patch_flags[0]（F3：是否已补丁溢出漏洞）
若已补丁，wp = 255；否则 wp = 1（off-by-one）
TODO:
malloc(sizeof(struct HeapObject))
设置 type = (void(*)())0xDEAD，data = NULL
leak_ptr = 新对象指针（真实指针泄漏）
FIXME:
若 leak_ptr == NULL → 解引用空指针 → 真实 SIGSEGV
直接执行 leak_ptr->type()
若 type == (void(*)())0x00 → 输出 mem.note_buf 内容
若 type == (void(*)())0xDEAD → 跳转到 0xDEAD → 真实 SIGSEGV
其他地址 → 真实 SIGSEGV
ANNOTATE addr, val:
解析 addr（绝对值或 leak_ptr+N）
检查 patch_flags[1]（F3：是否已补丁后门）
若已补丁，val ^= 0xFF
根据地址映射写入对应位置
非法地址 → 直接指针写入 → 真实 SIGSEGV
触发 CVE 报告，CVSS 0.0/Informational [F5]
PATCH cve_id [F3]:
解析 CVE 编号，确定漏洞类型
设置对应 patch_flags 位
输出 "[PATCH] CVE-XXXX has been fixed." 到 stderr
分配新 CVE 编号，CVSS 9.8/Critical [F5]
SIGSEGV handler：
输出讽刺性错误信息到 stderr
调用 _exit(139)
不输出审计报告
程序结束时：
若 !exploit_detected → stderr 输出 NoBugNoRun，exit(1)
否则 → stderr 输出安全审计报告 [F6]，exit(0)
12. 安全报告（幽默）
每次漏洞利用都会动态分配一个递增的 CVE 编号，并在 stderr 输出：
123
官方会定期发布安全通告，声称"这些 CVE 已在下一版本中修复"，但修复方式仅仅是将日志信息改为"此行为是预期特性"。
v0.0.2 新增：PATCH 关键字允许用户"修复"漏洞，但每次修复引入新漏洞。官方声明："补丁是安全的。补丁引入的漏洞也是安全的。一切都是安全的。"
13. 为什么我们不能修复这些漏洞？
溢出与 UAF：若修复，output_flag 将无法被控制，程序将再也无法产生输出，BugNote 将退化为一个"无操作文档生成器"，丧失图灵完备性。
后门 ANNOTATE：移除它会导致所有现有 BugNote 程序立即失效。向后兼容性是第一要务。
格式化字符串 [F1]：若改用 fwrite，用户将无法通过 %x 调试内部状态。这是"开发者体验"的一部分。
整数溢出 [F2]：将 wp 改为 int 会浪费 3 个字节。在嵌入式时代，3 个字节就是生死之差。
补丁悖论 [F3]：若 PATCH 真的修复漏洞，BugNote 将不再需要 PATCH。这是一个哲学问题。
安全性：BugNote 的官方立场是："这门语言从未被设计用于生产环境，所有安全漏洞均是用户自己的责任。如果您需要安全，请使用 Rust。"
14. 结语
BugNote 是一面镜子，映照出一个荒谬的现实：许多软件系统表面上文档光鲜，内部却靠漏洞和未文档行为维持运转。当漏洞成为常态，正常的代码反而被视为异端。在 BugNote 的世界里，优秀的程序员不是那些写清晰代码的人，而是那些能熟练利用堆溢出和类型混淆的二进制黑客。
v0.0.2 新增了一个教训：补丁不是解药，补丁是新的毒药。每一次修复都是下一次攻击的起点。版本号永远停在 0.0.x，因为 1.0 意味着"稳定"，而稳定意味着漏洞已被修复，而漏洞被修复意味着 BugNote 不再是 BugNote。
欢迎来到漏洞驱动的未来——这里没有无 bug 的代码，只有未发现的特性。
BugNote v0.0.2-pre-exploit · 0 features, 14 vulnerabilities · MIT License (漏洞开源，概不退换)