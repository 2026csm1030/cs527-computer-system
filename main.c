#include <stdio.h>
#include "compiler.h"
#include "memory.h"
#include "processor.h"

int main(void)
{
	printf("--- Starting Mini Computer Simulation ---\n");

	// Step 1: Translate input.txt into program.byte
	printf("[1/5] Compiling program...\n");
	compile();

	// Step 2: Load program.byte and data.byte into Instruction and Data arrays
	printf("[2/5] Initializing memory...\n");
	initialise();

	// Step 3: Reset processor registers and Program Counter (PC)
	printf("[3/5] Resetting processor...\n");
	reset();

	// Step 4: Execute the Fetch-Decode-Execute CPU cycle
	printf("[4/5] Executing simulation...\n");
	while (!end_of_simulation)
	{
		fetch();
		decode();
		execute();
	}

	// Step 5: Dump updated Data memory contents to data.byte
	printf("[5/5] Finalizing memory state...\n");
	finalize();

	printf("--- Simulation Finished Successfully ---\n");
	return 0;
}