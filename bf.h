#ifndef BRAINFUCK
#define BRAINFUCK

#define INITIAL_INSTRUCTION_SIZE 1024
#define INITIAL_OUTPUT_SIZE 1024

#define ARRAY_SIZE 32768
#include <stdbool.h>

typedef struct
{
	char *commands;
	int size;
	int reserved;
}commands;

typedef struct
{
	char *buffer;
	int length;
	int reserved;
}output;

typedef struct
{
	commands *com;
	output *output;	
	
	// Working memory
	char array[ARRAY_SIZE];
	int arrayPtr;
	
	// Command counter
	int commandCounter;
	
	// Input
	char *input;
	int inputSize;
	int inputPtr;
	
}environment;

void initCommand(commands *com);
bool addCommand(commands *com, char c);
void cleanupCommand(commands *com);
void copyCommand(commands *from, commands *to);
bool commandAvailable(environment *env);

void initOutput(output *out);
bool addOutput(output *out, char c);
void cleanupOutput(output *out);

void initEnvironment(environment *env);
unsigned int runEnvironment(environment *env, unsigned int maxInstructions);
void cleanupEnvironment(environment *env);

#endif