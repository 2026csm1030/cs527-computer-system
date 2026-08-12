#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

#define MAX_LABELS 100
#define MAX_LABEL_LEN 64

// Structure to hold label symbol table entries
typedef struct
{
  char name[MAX_LABEL_LEN];
  int instruction_index;
} LabelSymbol;

static LabelSymbol symbol_table[MAX_LABELS];
static int symbol_count = 0;

// Helper function to trim leading and trailing whitespace
static char *trim_whitespace(char *str)
{
  while (isspace((unsigned char)*str))
    str++;
  if (*str == 0)
    return str;

  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  end[1] = '\0';

  return str;
}

// Helper to look up branch suffix codes (0 to 14)
static int get_branch_suffix_code(const char *suffix)
{
  if (strcmp(suffix, "EQ") == 0)
    return 0;
  if (strcmp(suffix, "NE") == 0)
    return 1;
  if (strcmp(suffix, "CS") == 0)
    return 2;
  if (strcmp(suffix, "CC") == 0)
    return 3;
  if (strcmp(suffix, "MI") == 0)
    return 4;
  if (strcmp(suffix, "PL") == 0)
    return 5;
  if (strcmp(suffix, "VS") == 0)
    return 6;
  if (strcmp(suffix, "VC") == 0)
    return 7;
  if (strcmp(suffix, "HI") == 0)
    return 8;
  if (strcmp(suffix, "LS") == 0)
    return 9;
  if (strcmp(suffix, "GE") == 0)
    return 10;
  if (strcmp(suffix, "LT") == 0)
    return 11;
  if (strcmp(suffix, "GT") == 0)
    return 12;
  if (strcmp(suffix, "LE") == 0)
    return 13;
  if (strcmp(suffix, "AL") == 0)
    return 14;
  return -1;
}

// Find label in symbol table
static int find_label_index(const char *name)
{
  for (int i = 0; i < symbol_count; i++)
  {
    if (strcmp(symbol_table[i].name, name) == 0)
    {
      return symbol_table[i].instruction_index;
    }
  }
  return -1;
}

void compile(void)
{
  FILE *inFile = fopen("input.txt", "r");
  if (!inFile)
  {
    fprintf(stderr, "Error: Could not open input.txt\n");
    exit(EXIT_FAILURE);
  }

  char raw_line[256];
  symbol_count = 0;
  int current_inst_index = 0;

  // =========================================================================
  // PASS 1: Build Symbol Table for Labels
  // =========================================================================
  while (fgets(raw_line, sizeof(raw_line), inFile))
  {
    // Strip comments starting with '%'
    char *comment = strchr(raw_line, '%');
    if (comment)
      *comment = '\0';

    char *line = trim_whitespace(raw_line);
    if (strlen(line) == 0)
      continue;

    // Label definition (starts with '.')
    if (line[0] == '.')
    {
      if (symbol_count < MAX_LABELS)
      {
        strncpy(symbol_table[symbol_count].name, line, MAX_LABEL_LEN - 1);
        symbol_table[symbol_count].name[MAX_LABEL_LEN - 1] = '\0';
        symbol_table[symbol_count].instruction_index = current_inst_index;
        symbol_count++;
      }
    }
    else
    {
      // Regular instruction line
      current_inst_index++;
    }
  }

  // Reset file pointer to beginning for Pass 2
  fseek(inFile, 0, SEEK_SET);

  FILE *outFile = fopen("program.byte", "w");
  if (!outFile)
  {
    fprintf(stderr, "Error: Could not create program.byte\n");
    fclose(inFile);
    exit(EXIT_FAILURE);
  }

  current_inst_index = 0;

  // =========================================================================
  // PASS 2: Code Generation
  // =========================================================================
  while (fgets(raw_line, sizeof(raw_line), inFile))
  {
    // Strip comments starting with '%'
    char *comment = strchr(raw_line, '%');
    if (comment)
      *comment = '\0';

    char *line = trim_whitespace(raw_line);
    if (strlen(line) == 0 || line[0] == '.')
      continue; // Skip empty lines and labels

    int opcode = 0, dest = 0, op1 = 0, op2 = 0;

    int r_dest, r_op1, r_op2, const_val;
    char op_symbol;
    char label_target[MAX_LABEL_LEN];

    // ---------------------------------------------------------------------
    // 1. Branch Instructions (B<Suffix> <label> or BAL <label>)
    // ---------------------------------------------------------------------
    if (line[0] == 'B')
    {
      char branch_type[16];
      if (sscanf(line, "%s %s", branch_type, label_target) == 2)
      {
        const char *suffix = branch_type + 1; // Skip 'B'
        int code = get_branch_suffix_code(suffix);
        if (code != -1)
        {
          opcode = 16 + code;
          dest = 0;
          op1 = 0;

          int target_idx = find_label_index(label_target);
          if (target_idx == -1)
          {
            fprintf(stderr, "Error: Undefined label '%s'\n", label_target);
            exit(EXIT_FAILURE);
          }
          // Relative offset = target_index - current_instruction_index
          op2 = target_idx - current_inst_index;
        }
      }
    }
    // ---------------------------------------------------------------------
    // 2. Register Indirect Memory Write: [x2] = x1 or [0] = x1
    // ---------------------------------------------------------------------
    else if (line[0] == '[')
    {
      if (sscanf(line, "[x%d] = x%d", &r_dest, &r_op1) == 2)
      {
        opcode = 6; // Memory write (variable)
        dest = r_dest;
        op1 = r_op1;
        op2 = 0;
      }
      else if (sscanf(line, "[%d] = x%d", &const_val, &r_op1) == 2)
      {
        opcode = 14; // Memory write (constant address)
        dest = const_val;
        op1 = r_op1;
        op2 = 0;
      }
    }
    // ---------------------------------------------------------------------
    // 3. Read / Write legacy syntax support (Read x1, 0)
    // ---------------------------------------------------------------------
    else if (sscanf(line, "Read x%d, %d", &r_dest, &const_val) == 2)
    {
      opcode = 13;
      dest = r_dest;
      op1 = const_val;
      op2 = 0;
    }
    else if (sscanf(line, "Write x%d, %d", &r_dest, &const_val) == 2)
    {
      opcode = 14;
      dest = const_val;
      op1 = r_dest;
      op2 = 0;
    }
    // ---------------------------------------------------------------------
    // 4. Register Indirect Memory Read: x1 = [x3] or x1 = [0]
    // ---------------------------------------------------------------------
    else if (sscanf(line, "x%d = [x%d]", &r_dest, &r_op1) == 2)
    {
      opcode = 5; // Memory read (variable)
      dest = r_dest;
      op1 = r_op1;
      op2 = 0;
    }
    else if (sscanf(line, "x%d = [%d]", &r_dest, &const_val) == 2)
    {
      opcode = 13; // Memory read (constant address)
      dest = r_dest;
      op1 = const_val;
      op2 = 0;
    }
    // ---------------------------------------------------------------------
    // 5. Arithmetic operations: x1 = x2 + x3 OR x1 = x2 + 10
    // ---------------------------------------------------------------------
    else if (sscanf(line, "x%d = x%d %c x%d", &r_dest, &r_op1, &op_symbol, &r_op2) == 4)
    {
      dest = r_dest;
      op1 = r_op1;
      op2 = r_op2;
      if (op_symbol == '+')
        opcode = 1;
      else if (op_symbol == '-')
        opcode = 2;
      else if (op_symbol == '*')
        opcode = 3;
      else if (op_symbol == '/')
        opcode = 4;
    }
    else if (sscanf(line, "x%d = x%d %c %d", &r_dest, &r_op1, &op_symbol, &const_val) == 4)
    {
      dest = r_dest;
      op1 = r_op1;
      op2 = const_val;
      if (op_symbol == '+')
        opcode = 9;
      else if (op_symbol == '-')
        opcode = 10;
      else if (op_symbol == '*')
        opcode = 11;
      else if (op_symbol == '/')
        opcode = 12;
    }
    // ---------------------------------------------------------------------
    // 6. Data movement: x1 = x2 (Opcode 7) or x1 = 10 (Opcode 15 / 7)
    // ---------------------------------------------------------------------
    else if (sscanf(line, "x%d = x%d", &r_dest, &r_op1) == 2)
    {
      opcode = 7;
      dest = r_dest;
      op1 = r_op1;
      op2 = 0;
    }
    else if (sscanf(line, "x%d = %d", &r_dest, &const_val) == 2)
    {
      opcode = 7; // Or 15
      dest = r_dest;
      op1 = const_val;
      op2 = 0;
    }

    // Output bytecode line
    fprintf(outFile, "%d %d %d %d\n", opcode, dest, op1, op2);
    current_inst_index++;
  }

  // Append Halt instruction (Opcode 0)
  fprintf(outFile, "0 0 0 0\n");

  fclose(inFile);
  fclose(outFile);
  printf("Compilation complete. Bytecode generated in program.byte\n");
}