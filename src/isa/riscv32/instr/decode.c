/***************************************************************************************
* Copyright (c) 2014-2021 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "../local-include/rtl.h"
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <isa-all-instr.h>

def_all_THelper();

// decode operand helper
#define def_DopHelper(name) \
  void concat(decode_op_, name) (Decode *s, Operand *op, uint32_t val, bool flag)

static inline def_DopHelper(i) {
  op->imm = val;
  print_Dop(op->str, OP_STR_SIZE, (flag ? "0x%x" : "%d"), op->imm);
}

static inline def_DopHelper(r) {
  bool load_val = flag;
  static word_t zero_null = 0;
  op->preg = (!load_val && val == 0) ? &zero_null : &reg_l(val);
  print_Dop(op->str, OP_STR_SIZE, "%s", reg_name(val, 4));
}

static inline def_DHelper(I) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, true);
  decode_op_i(s, id_src2, s->isa.instr.i.simm11_0, false);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, false);
}

static inline def_DHelper(U) {
  decode_op_i(s, id_src1, s->isa.instr.u.imm31_12 << 12, true);
  decode_op_r(s, id_dest, s->isa.instr.u.rd, false);
}

static inline def_DHelper(S) {
  decode_op_r(s, id_src1, s->isa.instr.s.rs1, true);
  sword_t simm = (s->isa.instr.s.simm11_5 << 5) | s->isa.instr.s.imm4_0;
  decode_op_i(s, id_src2, simm, false);
  decode_op_r(s, id_dest, s->isa.instr.s.rs2, true);
}
#ifndef __ICS_EXPORT

static inline def_DHelper(R) {
  decode_op_r(s, id_src1, s->isa.instr.r.rs1, true);
  decode_op_r(s, id_src2, s->isa.instr.r.rs2, true);
  decode_op_r(s, id_dest, s->isa.instr.r.rd, false);
}

static inline def_DHelper(J) {
  sword_t offset = (s->isa.instr.j.simm20 << 20) | (s->isa.instr.j.imm19_12 << 12) |
    (s->isa.instr.j.imm11 << 11) | (s->isa.instr.j.imm10_1 << 1);
  decode_op_i(s, id_src1, s->pc + offset, true);
  decode_op_r(s, id_dest, s->isa.instr.j.rd, false);
  id_src2->imm = s->snpc;
}

static inline def_DHelper(B) {
  sword_t offset = (s->isa.instr.b.simm12 << 12) | (s->isa.instr.b.imm11 << 11) |
    (s->isa.instr.b.imm10_5 << 5) | (s->isa.instr.b.imm4_1 << 1);
  decode_op_i(s, id_dest, s->pc + offset, true);
  decode_op_r(s, id_src1, s->isa.instr.b.rs1, true);
  decode_op_r(s, id_src2, s->isa.instr.b.rs2, true);
}

static inline def_DHelper(auipc) {
  decode_U(s, width);
  id_src1->imm += s->pc;
}

static inline def_DHelper(csr) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, true);
  decode_op_i(s, id_src2, s->isa.instr.csr.csr, true);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, false);
}
#endif
