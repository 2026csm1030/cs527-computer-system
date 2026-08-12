#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

// define global instruction and data memory arrays
char Instruction[256];
int Data[4096];

void initialise(void)
{
  FILE *progFile = fopen("program.byte", "r");

  if (!progFile)
  {
    printf("Error: program.byte not found.\n");
    exit(EXIT_FAILURE);
  }

  // Populate Instruction Memory from program.byte
  int opcode, dest, op1, op2;
  int pc_index = 0;

  // Read 4 integers per line and store sequentially in Instruction array
  while (fscanf(progFile, "%d %d %d %d", &opcode, &dest, &op1, &op2) == 4)
  {
    if (pc_index + 3 < 256)
    {
      Instruction[pc_index] = (char)opcode;
      Instruction[pc_index + 1] = (char)dest;
      Instruction[pc_index + 2] = (char)op1;
      Instruction[pc_index + 3] = (char)op2;
      pc_index += 4; // Move to the next 4-byte instruction block
    }
  }
  fclose(progFile);

  FILE *dataFile = fopen("data.byte", "r");

  if (!dataFile)
  {
    printf("Error: data.byte not found.\n");
    exit(EXIT_FAILURE);
  }

  // populate Data Memory from data.byte

  int val;
  int data_index = 0;

  while (fscanf(dataFile, "%d", &val) == 1 && data_index < 4096)
  {
    Data[data_index] = (char)val;
    data_index++;
  }
  fclose(dataFile);
}

void finalize(void)
{
  // Write modified Data memory array back to data.byte
  FILE *dataFile = fopen("data.byte", "w");

  if (dataFile)
  {
    for (int i = 0; i < 4096; i++)
    {
      fprintf(dataFile, "%d\n", Data[i]);
    }

    fclose(dataFile);
    printf("Memory state successfully written to data.byte\n");
  }
  else
  {
    printf("Error: Could not save data.byte\n");
    exit(EXIT_FAILURE);
  }
}