#!/bin/bash
#*******************************************************************************
# Copyright (c) 2026 RV32 NEMU Project
#
# RV32 NEMU 快速编译脚本
# 用于快速编译32位RISC-V版本的NEMU模拟器
#
# Usage:
#   ./build_rv32.sh          # 编译共享库（默认，用于Difftest）
#   ./build_rv32.sh exe      # 编译独立可执行文件
#   ./build_rv32.sh clean    # 清理编译产物
#   ./build_rv32.sh all      # 同时编译共享库和可执行文件
#*******************************************************************************

set -e

# 设置NEMU_HOME
export NEMU_HOME=$(cd "$(dirname "$0")" && pwd)
echo "NEMU_HOME = $NEMU_HOME"

# 检查配置文件
if [ ! -f "$NEMU_HOME/.config" ]; then
    echo "⚠️  未找到配置文件，使用默认RV32配置..."
    make -C "$NEMU_HOME" riscv32-xs_defconfig
fi

# 检查是否为RV32配置
if ! grep -q "CONFIG_ISA_riscv32=y" "$NEMU_HOME/.config"; then
    echo "❌ 当前配置不是RV32!"
    echo "请先运行: make riscv32-xs_defconfig"
    exit 1
fi

echo "✅ 确认RV32配置"

# 编译函数
build_shared() {
    echo ""
    echo "=========================================="
    echo "  编译RV32 NEMU共享库 (Difftest)"
    echo "=========================================="
    make -C "$NEMU_HOME" -j$(nproc) SHARE=1
    echo ""
    echo "✅ 编译完成!"
    ls -lh "$NEMU_HOME/build/riscv32-nemu-interpreter-so"
}

build_exe() {
    echo ""
    echo "=========================================="
    echo "  编译RV32 NEMU独立可执行文件"
    echo "=========================================="
    make -C "$NEMU_HOME" -j$(nproc)
    echo ""
    echo "✅ 编译完成!"
    ls -lh "$NEMU_HOME/build/riscv32-nemu-interpreter"
}

clean_build() {
    echo ""
    echo "=========================================="
    echo "  清理编译产物"
    echo "=========================================="
    make -C "$NEMU_HOME" clean
    echo "✅ 清理完成"
}

# 根据参数执行
case "${1:-so}" in
    so|shared|lib)
        build_shared
        ;;
    exe|bin|exec)
        build_exe
        ;;
    clean)
        clean_build
        ;;
    all)
        build_shared
        build_exe
        ;;
    help|--help|-h)
        echo "RV32 NEMU 编译脚本"
        echo ""
        echo "用法:"
        echo "  $0          编译共享库 (默认，用于Difftest)"
        echo "  $0 so       编译共享库"
        echo "  $0 exe      编译独立可执行文件"
        echo "  $0 clean    清理编译产物"
        echo "  $0 all      编译共享库和可执行文件"
        echo "  $0 help     显示帮助信息"
        echo ""
        echo "编译产物位置: build/"
        ;;
    *)
        echo "未知参数: $1"
        echo "运行 $0 help 查看帮助"
        exit 1
        ;;
esac

echo ""
echo "=========================================="
echo "  使用说明"
echo "=========================================="
echo "📚 详细文档请参考: RV32_BUILD_GUIDE.md"
echo ""
echo "💡 Difftest中使用:"
echo "   export REF_SO=$NEMU_HOME/build/riscv32-nemu-interpreter-so"
echo "   ./simulator --diff \$REF_SO"
echo ""
