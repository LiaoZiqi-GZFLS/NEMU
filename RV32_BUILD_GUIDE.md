# RV32 NEMU 编译与使用指南

本文档记录了如何从XiangShan NEMU仓库编译RV32（32位RISC-V）版本的NEMU模拟器，作为差分测试（Difftest）的黄金参考模型。

## 目录

1. [背景说明](#背景说明)
2. [环境要求](#环境要求)
3. [编译步骤](#编译步骤)
4. [编译产物](#编译产物)
5. [使用方法](#使用方法)
6. [支持的功能](#支持的功能)
7. [故障排除](#故障排除)

---

## 背景说明

XiangShan NEMU（NJU Emulator）主要支持RV64（64位RISC-V）架构。为了支持RV32架构的CPU设计进行差分测试，需要对代码进行多处修改以适配32位架构。本文档记录了完整的编译过程。

**适用场景**：
- RV32I/RV32IM/RV32IMZ CPU设计的差分测试
- 32位RISC-V程序仿真验证
- 作为Difftest框架的黄金参考模型

---

## 环境要求

### 操作系统
- Ubuntu 18.04+ / Debian 10+ 或其他兼容Linux发行版
- WSL 2 (Windows Subsystem for Linux) 也支持

### 依赖软件
```bash
# Ubuntu/Debian
sudo apt-get install build-essential gcc g++ make libz-dev

# 验证版本
gcc --version  # 建议 7.5+
g++ --version  # 建议 7.5+
make --version # 建议 4.1+
```

---

## 编译步骤

### 步骤1：配置RV32编译选项

首次编译需要运行配置菜单：

```bash
cd /path/to/NEMU
export NEMU_HOME=$(pwd)
make riscv32-xs_defconfig
```

**重要配置项**：
- `ISA`: 选择 `riscv32`
- `Engine`: 选择 `interpreter`（解释器模式）
- `Differential testing`: 根据需要启用
- `RV64`: **禁用**（确保不选中）

### 步骤2：编译

#### 方式1：编译独立可执行文件（用于简单测试）
```bash
export NEMU_HOME=$(pwd)
make -j$(nproc)
```

#### 方式2：编译差分测试用的共享库（推荐）
```bash
export NEMU_HOME=$(pwd)
make -j$(nproc) SHARE=1
```

#### 清理重新编译
```bash
make clean
# 或深度清理
make distclean
```

### 步骤3：验证编译结果
```bash
# 检查生成的文件
ls -lh build/riscv32-nemu-interpreter
ls -lh build/riscv32-nemu-interpreter-so
```

---

## 编译产物

编译成功后，`build/` 目录下会生成以下文件：

| 文件 | 大小 | 用途 |
|------|------|------|
| `riscv32-nemu-interpreter` | ~2.2 MB | 独立可执行模拟器，可直接运行RV32程序 |
| `riscv32-nemu-interpreter-so` | ~2.6 MB | 共享库，用于Difftest差分测试框架 |

---

## 使用方法

### 1. 作为Difftest黄金参考模型

在你的CPU项目的Difftest配置中，指定NEMU共享库路径：

```bash
# 在你的CPU项目的仿真脚本中
export NEMU_HOME=/path/to/NEMU
export REF_SO=$NEMU_HOME/build/riscv32-nemu-interpreter-so

# 运行Verilator仿真时传入参数
./simulator --diff $REF_SO
```

**Difftest集成要点**：
- 确保你的RV32 CPU设计的寄存器布局与NEMU一致（32个通用寄存器 + pc）
- 寄存器字节序需要匹配（小端序）
- 异常处理行为需要对齐

### 2. 独立运行RV32程序

```bash
# 运行编译好的ELF文件
./build/riscv32-nemu-interpreter -b your_program.elf

# 带调试信息运行
./build/riscv32-nemu-interpreter -l your_program.elf

# 查看帮助
./build/riscv32-nemu-interpreter --help
```

### 3. 集成到Makefile示例

```makefile
NEMU_HOME ?= /path/to/NEMU
REF_SO    = $(NEMU_HOME)/build/riscv32-nemu-interpreter-so

# 编译NEMU参考模型
$(REF_SO):
	$(MAKE) -C $(NEMU_HOME) SHARE=1 -j$$(nproc)

# 仿真依赖参考模型
sim: $(REF_SO)
	$(VERILATOR) --diff $(REF_SO) ...
```

---

## 支持的功能

### 已支持的指令集
- ✅ RV32I 基础整数指令集
- ✅ RV32M 乘除法扩展
- ✅ 系统指令（ecall, ebreak, sret, sfence.vma）
- ✅ CSR 寄存器访问（csrrw, csrrs, csrrc）

### 已支持的特权模式功能
- ✅ Machine Mode / Supervisor Mode
- ✅ 异常处理
- ✅ 中断处理（基础框架）
- ✅ 虚拟内存翻译（SV32）

### 注意：未完全支持的功能
以下功能为简化实现，如需要完整支持需进一步开发：
- ⚠️ 浮点扩展（RV32F/RV32D）- 仅存根函数
- ⚠️ 向量扩展（RV32V）
- ⚠️ 所有CSR寄存器完整模拟
- ⚠️ 性能计数器

---

## 关键修改点记录

为了让RV32版本成功编译，对源码进行了以下修改：

### 1. CPU状态结构扩展 (`src/isa/riscv32/include/isa-def.h`)
- 添加 `mode`, `amo`, `pbmt`, `isVldst` 字段
- 添加 `NonRegInterruptPending` 结构体
- 添加 `trapInfo` 字段

### 2. 异常和中断头文件 (`src/isa/riscv32/local-include/intr.h`)
- 补充完整的异常码定义（EX_II等）

### 3. 新增/修改的函数
- `isa_hostcall()` - 修正函数签名匹配
- `isa_fetch_decode()` - 指令取指与解码函数
- `isa_pma_check_permission()` - 物理内存属性检查
- `isa_misalign_data_addr_check()` - 地址不对齐检查
- `able_to_take_cpt()` - Checkpoint支持函数
- 各种FP浮点和difftest存根函数

### 4. 指令表更新 (`src/isa/riscv32/include/isa-all-instr.h`)
- 补充 `csrrc` 指令到指令表

### 5. 格式字符串修复
- 多处 `%lx` 改为 `FMT_WORD` 宏以适配32位

---

## 故障排除

### 问题1：`NEMU_HOME not set` 错误
```bash
# 解决：执行前先设置环境变量
export NEMU_HOME=$(pwd)
make ...
```

### 问题2：编译时出现大量格式字符串错误
检查你的编译器版本，确保使用GCC而非clang。RV32的32位指针格式需要正确处理。

### 问题3：链接时找不到函数
确认所有存根函数已添加到以下文件：
- `src/isa/riscv32/init.c` - 主要函数
- `src/isa/riscv32/difftest/ref.c` - 差分测试和FP相关函数

### 问题4：Difftest时寄存器不匹配
1. 检查你的CPU复位后初始状态是否与NEMU一致
2. 确认寄存器字节序（小端序）
3. 检查pc的递增是否正确（4字节对齐）

### 问题5：共享库无法加载
确认使用 `SHARE=1` 参数编译，并且架构匹配（都是32位或都是64位主机）。

---

## 性能说明

- 编译时间（4核CPU）：约1-2分钟
- 仿真速度：约1-5 MIPS（取决于主机性能）
- 内存占用：运行时约10-50 MB

---

## 进一步优化建议

如果需要更完整的RV32支持，可以：

1. **实现完整的CSR寄存器**：参考RV64的 `csr.h` 和相关实现
2. **添加浮点支持**：实现RV32F浮点操作
3. **完善MMU**：实现完整的SV32页表翻译
4. **调试功能**：添加GDB远程调试支持

---

## 相关资源

- [XiangShan官方仓库](https://github.com/OpenXiangShan/NEMU)
- [RISC-V特权架构手册](https://riscv.org/technical/specifications/)
- [RV32指令集手册](https://riscv.org/technical/specifications/)

---

**文档版本**：1.0  
**最后更新**：2026-04-25  
**适用NEMU版本**：XiangShan NEMU master分支
