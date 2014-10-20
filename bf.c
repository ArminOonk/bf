#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define INITIAL_INSTRUCTION_SIZE 1024
#define INITIAL_OUTPUT_SIZE 1024
#define INITIAL_STACK_SIZE 128

#define ARRAY_SIZE 32768
typedef struct
{
	char *commands;
	int pointer;
	int size;
}commands;

void initCommand(commands *com)
{
	com->commands = NULL;
	com->pointer = 0;
	com->size = 0;
}

bool addCommand(commands *com, char c)
{
	if(com->pointer >= com->size)
	{
		// Re allocate memory
		int newBufferSize = com->size+INITIAL_INSTRUCTION_SIZE;
		//printf("Allocating memory. Current size: %d New size: %d\n", com->size, newBufferSize);
		
		com->commands = realloc(com->commands, newBufferSize);
		if(com->commands == NULL)
		{
			return false;
		}
		com->size = newBufferSize;
	}
	com->commands[com->pointer] = c;
	com->pointer++;
	return true;
}

void printCommand(commands *com)
{
	printf("Size: %d Size: %d\n", com->pointer, com->size);
	for(int i=0; i<com->pointer; i++)
	{
		printf("%c", com->commands[i]);
	}
	printf("\n");
}

void cleanupCommand(commands *com)
{
	free(com->commands);
}

typedef struct
{
	int *stack;
	int pointer;
	int size;
}stack;

void initStack(stack *st)
{
	st->stack = NULL;
	st->pointer = 0;
	st->size = 0;
}

bool pushStack(stack *st, int index)
{
	if(st->pointer >= st->size)
	{
		int newBufferSize = st->size+INITIAL_STACK_SIZE;
		st->stack = realloc(st->stack, sizeof(int)*newBufferSize);
		if(st->stack == NULL)
		{
			return false;
		}
		st->size = newBufferSize;
	}
	st->stack[st->pointer] = index;
	st->pointer++;
	return true;
}

int popStack(stack *st)
{
	if(st->pointer <= 0)
	{
		return -1;
	}
	
	int retVal = st->stack[st->pointer];
	st->pointer--;
	return retVal;
}

void cleanupStack(stack *st)
{
	free(st->stack);
}

typedef struct
{
	char *buffer;
	int pointer;
	int size;
}output;

void initOutput(output *out)
{
	out->buffer = NULL;
	out->pointer = 0;
	out->size = 0;
}

bool addOutput(output *out, char c)
{
	if(out->pointer >= out->size)
	{
		// Re allocate memory
		int newBufferSize = out->size+INITIAL_OUTPUT_SIZE;
		
		out->buffer = realloc(out->buffer, newBufferSize);
		if(out->buffer == NULL)
		{
			return false;
		}
		out->size = newBufferSize;
	}
	out->buffer[out->pointer] = c;
	out->pointer++;
	return true;
}

void cleanupOutput(output *out)
{
	free(out->buffer);
}

typedef struct
{
	stack *st;
	commands *com;
	output *output;	
	
	char array[ARRAY_SIZE];
	int pointer;
	
	int commandCounter;
}environment;

void initEnvironment(environment *env)
{
	env->pointer = 0;
	memset(env->array, 0x00, ARRAY_SIZE);
	
	env->commandCounter = 0;
	
	env->st = malloc(sizeof(stack));
	env->com = malloc(sizeof(commands));
	env->output = malloc(sizeof(output));
	
	initStack(env->st);
	initCommand(env->com);
	initOutput(env->output);
}

void runEnvironment(environment *env)
{
	bool finished = false;
	
	while(!finished)
	{
		if(env->pointer >= env->com->size)
		{
			break;
		}
		
		// ERROR env->pointer is niet de pointer naar de command list
		switch(env->com->commands[env->commandCounter])
		{
			case '>':
			env->pointer++;
			
			if(env->pointer >= ARRAY_SIZE)
			{
				env->pointer = 0;
			}
			break;
			
			case '<':
			env->pointer--;
						
			if(env->pointer < 0)
			{
				env->pointer = ARRAY_SIZE-1;
			}
			break;
			
			case '+':
				env->array[env->pointer]++;
			break;
			
			case '-':
				env->array[env->pointer]--;
			break;
			
			case '.':
				addOutput(env->output, env->array[env->pointer]);
			break;
			
			case ',':
				// TODO: Implement this
			break;
			
			case '[':
				/*if(env->array[env->pointer] == 0)
				{
					//Find closing bracket
					while(env->commandCounter <= env->com->size && env->com->commands[env->commandCounter] != ']')
					{
						env->commandCounter++;
					}
				}*/
			break;
			
			case ']':
			
			break;
		}
		
		env->commandCounter++;
	}
}

void cleanupEnvironment(environment *env)
{
	cleanupStack(env->st);
	cleanupCommand(env->com);
	cleanupOutput(env->output);

	free(env->st);
	free(env->com);
	free(env->output);
}

int readFile(char *filename, commands *com)
{
	FILE *readFP = fopen(filename, "r");
	if ( readFP == NULL )
	{
		printf("Could not read input file: %s\n", filename);
		return -1;
	}
	
	int x;
	int index = 0;
	
	while  ((x = fgetc( readFP ) ) != EOF )
	{
		switch(x)
		{
			case '>':
			addCommand(com, x);
			break;
			
			case '<':
			addCommand(com, x);
			break;
			
			case '+':
			addCommand(com, x);
			break;
			
			case '-':
			addCommand(com, x);
			break;
			
			case '.':
			addCommand(com, x);
			break;
			
			case ',':
			addCommand(com, x);
			break;
			
			case '[':
			addCommand(com, x);
			break;
			
			case ']':
			addCommand(com, x);
			break;
			
			default:
			printf("Not a brainfuck command: %c\n", x);
			break;
		}
		index++;
	}
	
	fclose(readFP);
	return index;
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		return -1;
	}
	
	environment env;
	initEnvironment(&env);
		
	if(readFile(argv[1], env.com) < 0)
	{
		printf("Error reading file\n");
	}
	
	printCommand(env.com);
	
	
	cleanupEnvironment(&env);
}