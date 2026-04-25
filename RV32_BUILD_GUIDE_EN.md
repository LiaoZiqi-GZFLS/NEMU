# RV32 NEMU Build and Usage Guide

This document describes how to build the RV32 (32-bit RISC-V) version of the NEMU emulator from the XiangShan repository, and use it as a golden reference model for differential testing (Difftest).

## Table of Contents

1. [Background](#background)
2. [Requirements](#requirements)
3. [Build Instructions](#build-instructions)
4. [Build Outputs](#build-outputs)
5. [Usage](#usage)
6. [Supported Features](#supported-features)
7. [Troubleshooting](#troubleshooting)

---

## Background

XiangShan NEMU (NJU Emulator) primarily supports RV64 (64-bit RISC-V) architecture. To support differential testing for RV32 CPU designs, numerous code modifications were needed to adapt to the 32-bit architecture. This document records the complete build process.

**Use Cases**:
- Differential testing for RV32I/RV32IM/RV32IMZ CPU designs
- 32-bit RISC-V program simulation and verification
- Golden reference model for Difftest framework

---

## Requirements

### Operating System
- Ubuntu 18.04+ / Debian 10+ or compatible Linux distribution
- WSL 2 (Windows Subsystem for Linux) is also supported

### Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install build-essential gcc g++ make libz-dev

# Verify versions
gcc --version  # 7.5+ recommended
g++ --version  # 7.5+ recommended
make --version # 4.1+ recommended
```

---

## Build Instructions

### Step 1: Configure RV32 Build Options

For the first build, run the configuration menu:

```bash
cd /path/to/NEMU
export NEMU_HOME=$(pwd)
make riscv32-xs_defconfig
```

**Important Configuration Items**:
- `ISA`: Select `riscv32`
- `Engine`: Select `interpreter` mode
- `Differential testing`: Enable as needed
- `RV64`: **DISABLE** (ensure not selected)

### Step 2: Compile

#### Option 1: Build standalone executable (for simple testing)
```bash
export NEMU_HOME=$(pwd)
make -j$(nproc)
```

#### Option 2: Build shared library for Difftest (recommended)
```bash
export NEMU_HOME=$(pwd)
make -j$(nproc) SHARE=1
```

#### Clean and Rebuild
```bash
make clean
# Or deep clean
make distclean
```

### Step 3: Verify Build Results
```bash
# Check generated files
ls -lh build/riscv32-nemu-interpreter
ls -lh build/riscv32-nemu-interpreter-so
```

---

## Build Outputs

After successful compilation, the following files will be generated in the `build/` directory:

| File | Size | Purpose |
|------|------|---------|
| `riscv32-nemu-interpreter` | ~2.2 MB | Standalone executable emulator, runs RV32 programs directly |
| `riscv32-nemu-interpreter-so` | ~2.6 MB | Shared library for Difftest differential testing framework |

---

## Usage

### 1. As Difftest Golden Reference Model

In your CPU project's Difftest configuration, specify the NEMU shared library path:

```bash
# In your CPU project's simulation script
export NEMU_HOME=/path/to/NEMU
export REF_SO=$NEMU_HOME/build/riscv32-nemu-interpreter-so

# Pass parameter when running Verilator simulation
./simulator --diff $REF_SO
```

**Difftest Integration Notes**:
- Ensure your RV32 CPU design's register layout matches NEMU (32 general registers + pc)
- Register endianness must match (little-endian)
- Exception handling behavior must be aligned

### 2. Run RV32 Programs Standalone

```bash
# Run compiled ELF file
./build/riscv32-nemu-interpreter -b your_program.elf

# Run with debug information
./build/riscv32-nemu-interpreter -l your_program.elf

# View help
./build/riscv32-nemu-interpreter --help
```

### 3. Makefile Integration Example

```makefile
NEMU_HOME ?= /path/to/NEMU
REF_SO    = $(NEMU_HOME)/build/riscv32-nemu-interpreter-so

# Build NEMU reference model
$(REF_SO):
	$(MAKE) -C $(NEMU_HOME) SHARE=1 -j$$(nproc)

# Simulation depends on reference model
sim: $(REF_SO)
	$(VERILATOR) --diff $(REF_SO) ...
```

---

## Supported Features

### Supported Instruction Sets
- ✅ RV32I base integer instruction set
- ✅ RV32M multiplication and division extension
- ✅ System instructions (ecall, ebreak, sret, sfence.vma)
- ✅ CSR register access (csrrw, csrrs, csrrc)

### Supported Privileged Mode Features
- ✅ Machine Mode / Supervisor Mode
- ✅ Exception handling
- ✅ Interrupt handling (basic framework)
- ✅ Virtual memory translation (SV32)

### Note: Partially Supported Features
The following features have simplified implementations. Full support requires further development:
- ⚠️ Floating-point extensions (RV32F/RV32D) - stub functions only
- ⚠️ Vector extension (RV32V)
- ⚠️ Complete CSR register emulation
- ⚠️ Performance counters

---

## Key Modifications Record

To successfully compile the RV32 version, the following modifications were made to the source code:

### 1. CPU State Structure Extension (`src/isa/riscv32/include/isa-def.h`)
- Added `mode`, `amo`, `pbmt`, `isVldst` fields
- Added `NonRegInterruptPending` structure
- Added `trapInfo` field

### 2. Exception and Interrupt Headers (`src/isa/riscv32/local-include/intr.h`)
- Supplemented complete exception code definitions (EX_II, etc.)

### 3. New/Modified Functions
- `isa_hostcall()` - fixed function signature matching
- `isa_fetch_decode()` - instruction fetch and decode function
- `isa_pma_check_permission()` - physical memory attribute check
- `isa_misalign_data_addr_check()` - address misalignment check
- `able_to_take_cpt()` - checkpoint support function
- Various FP floating-point and difftest stub functions

### 4. Instruction Table Update (`src/isa/riscv32/include/isa-all-instr.h`)
- Added `csrrc` instruction to the instruction table

### 5. Format String Fixes
- Changed multiple `%lx` to `FMT_WORD` macro for 32-bit compatibility

---

## Troubleshooting

### Issue 1: `NEMU_HOME not set` Error
```bash
# Solution: Set environment variable before execution
export NEMU_HOME=$(pwd)
make ...
```

### Issue 2: Numerous Format String Errors During Compilation
Check your compiler version, ensure GCC is used rather than clang. RV32's 32-bit pointer format requires correct handling.

### Issue 3: Functions Not Found During Linking
Verify all stub functions have been added to:
- `src/isa/riscv32/init.c` - main functions
- `src/isa/riscv32/difftest/ref.c` - difftest and FP-related functions

### Issue 4: Register Mismatch During Difftest
1. Check if your CPU's initial state after reset matches NEMU
2. Verify register endianness (little-endian)
3. Check if pc increment is correct (4-byte aligned)

### Issue 5: Shared Library Cannot Load
Ensure compiled with `SHARE=1` parameter, and architecture matches (all 32-bit or all 64-bit host).

---

## Performance Notes

- Compilation time (4-core CPU): ~1-2 minutes
- Simulation speed: ~1-5 MIPS (depending on host performance)
- Memory footprint: ~10-50 MB runtime

---

## Further Optimization Suggestions

If more complete RV32 support is needed, you can:

1. **Implement complete CSR registers**: Reference RV64's `csr.h` and related implementations
2. **Add floating-point support**: Implement RV32F floating-point operations
3. **Complete MMU**: Implement full SV32 page table translation
4. **Debug features**: Add GDB remote debugging support

---

## Related Resources

- [XiangShan Official Repository](https://github.com/OpenXiangShan/NEMU)
- [RISC-V Privileged Architecture Manual](https://riscv.org/technical/specifications/)
- [RV32 Instruction Set Manual](https://riscv.org/technical/specifications/)

---

**Document Version**: 1.0  
**Last Updated**: 2026-04-25  
**Applicable NEMU Version**: XiangShan NEMU master branch
