#include <stdio.h>
#include "processor.h"
#include "memory.h"

int Register[256];
int PC = 0;
int opcode = 0, dest = 0, src1 = 0, src2 = 0;
int end_of_simulation = 0;

int N = 0, Z = 0, C = 0, V = 0;

// Resets registers and Program Counter to 0
void reset(void)
{
  for (int i = 0; i < 256; i++)
  {
    Register[i] = 0;
  }
  PC = 0;
  opcode = 0;
  dest = 0;
  src1 = 0;
  src2 = 0;
  end_of_simulation = 0;

  N = 0;
  Z = 0;
  C = 0;
  V = 0;
}

// Reads 4 bytes sequentially from Instruction memory using PC offset
void fetch(void)
{
  opcode = (unsigned char)Instruction[PC];
  dest = (unsigned char)Instruction[PC + 1];
  src1 = (unsigned char)Instruction[PC + 2];

  // src2 = (unsigned char)Instruction[PC + 3];
  // src2 can hold a signed relative offset for branches
  src2 = (signed char)Instruction[PC + 3];

  // Increment PC by 4 (4-byte instruction width)
  PC += 4;
}

void decode(void) {}

static void update_add_flags(int a, int b, int res)
{
  // Z: 1 if result is zero
  Z = (res == 0) ? 1 : 0;

  // N: 1 if MSB of 32-bit result is set (negative)
  N = (res < 0) ? 1 : 0;

  // C: 1 if unsighed addition wrapped around (signed overflow)
  C = ((unsigned int)res < (unsigned int)a) ? 1 : 0;

  // V: 1 if operands have same sign, but result sign differs
  int a_sign = (a < 0);
  int b_sign = (b < 0);
  int res_sign = (res < 0);
  V = (a_sign == b_sign && res_sign != a_sign) ? 1 : 0;
}

// Helper function to update NZCV flags for subtraction
static void update_sub_flags(int a, int b, int res)
{
  // Z: 1 if result is zero
  Z = (res == 0) ? 1 : 0;

  // N: 1 if MSB of 32-bit result is set (negative)
  N = (res < 0) ? 1 : 0;

  // C: 1 if operand 1 >= operand 2 (unsigned)
  C = ((unsigned int)a >= (unsigned int)b) ? 1 : 0;

  // V: 1 if operands have different signs, and result sign matches operand 2's sign
  int a_sign = (a < 0);
  int b_sign = (b < 0);
  int res_sign = (res < 0);
  V = (a_sign != b_sign && res_sign == b_sign) ? 1 : 0;
}

// Executes instruction based on decoded opcode
void execute(void)
{
  if (opcode == 0)
  {
    end_of_simulation = 1;
    return;
  }

  int res = 0;

  switch (opcode)
  {
  // -------------------------------------------------------------
  // Arithmetic Operations (Variable vs Constant)
  // -------------------------------------------------------------
  case 1: // ADD (Register + Register)
    res = Register[src1] + Register[src2];
    update_add_flags(Register[src1], Register[src2], res);
    Register[dest] = res;
    break;

  case 9: // ADD (Register + Immediate Constant)
    res = Register[src1] + src2;
    update_add_flags(Register[src1], src2, res);
    Register[dest] = res;
    break;

  case 2: // SUB (Register - Register)
    res = Register[src1] - Register[src2];
    update_sub_flags(Register[src1], Register[src2], res);
    Register[dest] = res;
    break;

  case 10: // SUB (Register - Immediate Constant)
    res = Register[src1] - src2;
    update_sub_flags(Register[src1], src2, res);
    Register[dest] = res;
    break;

  case 3: // MUL (Register * Register)
    Register[dest] = Register[src1] * Register[src2];
    break;

  case 11: // MUL (Register * Immediate Constant)
    Register[dest] = Register[src1] * src2;
    break;

  case 4: // DIV (Register / Register)
    if (Register[src2] != 0)
      Register[dest] = Register[src1] / Register[src2];
    break;

  case 12: // DIV (Register / Immediate Constant)
    if (src2 != 0)
      Register[dest] = Register[src1] / src2;
    break;

  // -------------------------------------------------------------
  // Memory Operations (Read / Write)
  // -------------------------------------------------------------
  case 5: // Memory Read (Variable Address): Register[dest] = [Register[src1]]
    Register[dest] = Data[Register[src1]];
    break;

  case 13: // Memory Read (Constant Address): Register[dest] = [src1]
    Register[dest] = Data[src1];
    break;

  case 6: // Memory Write (Variable Address): [Register[dest]] = Register[src1]
    Data[Register[dest]] = (char)Register[src1];
    break;

  case 14: // Memory Write (Constant Address): [dest] = Register[src1]
    Data[dest] = (char)Register[src1];
    break;

  // -------------------------------------------------------------
  // Data Movement Operations
  // -------------------------------------------------------------
  case 7: // Register = Register OR Register = Immediate
  case 15:
    Register[dest] = src1;
    break;

  // -------------------------------------------------------------
  // Control Operations (Branching Opcodes 16 to 30)
  // -------------------------------------------------------------
  default:
    if (opcode >= 16 && opcode <= 30)
    {
      int condition_met = 0;
      int code = opcode - 16;

      switch (code)
      {
      case 0:
        condition_met = (Z == 1);
        break; // EQ
      case 1:
        condition_met = (Z == 0);
        break; // NE
      case 2:
        condition_met = (C == 1);
        break; // CS
      case 3:
        condition_met = (C == 0);
        break; // CC
      case 4:
        condition_met = (N == 1);
        break; // MI
      case 5:
        condition_met = (N == 0);
        break; // PL
      case 6:
        condition_met = (V == 1);
        break; // VS
      case 7:
        condition_met = (V == 0);
        break; // VC
      case 8:
        condition_met = (C == 1 && Z == 0);
        break; // HI
      case 9:
        condition_met = (C == 0 || Z == 1);
        break; // LS
      case 10:
        condition_met = (N == V);
        break; // GE
      case 11:
        condition_met = (N != V);
        break; // LT
      case 12:
        condition_met = (Z == 0 && (N == V));
        break; // GT
      case 13:
        condition_met = (Z == 1 || (N != V));
        break; // LE
      case 14:
        condition_met = 1;
        break; // AL
      }

      // If condition is satisfied, update PC using relative offset (src2)
      if (condition_met)
      {
        int current_inst_index = (PC / 4) - 1;
        int target_inst_index = current_inst_index + src2;
        PC = target_inst_index * 4;
      }
    }
    break;
  }
}