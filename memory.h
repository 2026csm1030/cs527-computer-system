#ifndef MEMORY_H
#define MEMORY_H

extern char Instruction[256];

extern int Data[4096]; // data memory expand from 256 bytes to 4096 bytes

void initialise(void);
void finalize(void);

#endif