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
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <difftest.h>
#include "../local-include/intr.h"

char *reg_dump_file = NULL;

// FP stubs
int isa_fp_csr_check(void) { return 0; }
void isa_fp_set_ex(int ex) { }
int isa_fp_get_rm(void) { return 0; }
int isa_fp_rm_check(int rm) { return 0; }
int isa_fp_get_frm(void) { return 0; }

// Difftest stubs
void isa_difftest_csrcpy(void *dut, bool direction) { }
void isa_difftest_uarchstatus_cpy(void *dut, bool direction) { }
void isa_update_mip(unsigned lcofip) { }
void isa_sync_custom_mflushpwr(bool l2FlushDone) { }

void isa_difftest_regcpy(void *dut, bool direction) {
  if (direction == DIFFTEST_TO_REF) memcpy(&cpu, dut, DIFFTEST_REG_SIZE);
  else memcpy(dut, &cpu, DIFFTEST_REG_SIZE);
}

void isa_difftest_raise_intr(word_t NO) {
  cpu.pc = raise_intr(NO, cpu.pc);
}
