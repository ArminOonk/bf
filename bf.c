#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define INITIAL_INSTRUCTION_SIZE 1024
#define INITIAL_OUTPUT_SIZE 1024

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
	printf("Size: %d Commands BufferSize: %d\n", com->pointer, com->size);
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
	char *buffer;
	int length;
	int reserved;
}output;

void initOutput(output *out)
{
	out->buffer = NULL;
	out->length = 0;
	out->reserved = 0;
}

bool addOutput(output *out, char c)
{
	if(out->length >= out->reserved)
	{
		// Re allocate memory
		int newBufferSize = out->reserved+INITIAL_OUTPUT_SIZE;
		
		out->buffer = realloc(out->buffer, newBufferSize);
		if(out->buffer == NULL)
		{
			return false;
		}
		out->reserved = newBufferSize;
	}
	out->buffer[out->length] = c;
	out->length++;
	return true;
}

void printOutput(output *out)
{
	printf("Output size %d : ", out->length);
	for(int i=0; i<out->length; i++)
	{
		printf("%c", out->buffer[i]);
	}
	printf("\n");
}

void cleanupOutput(output *out)
{
	free(out->buffer);
}

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

void initEnvironment(environment *env)
{
	env->arrayPtr = 0;
	memset(env->array, 0x00, ARRAY_SIZE);
	
	env->commandCounter = 0;

	env->com = malloc(sizeof(commands));
	env->output = malloc(sizeof(output));

	initCommand(env->com);
	initOutput(env->output);
	
	env->input = NULL;
	env->inputSize = 0;
	env->inputPtr = 0;
}

bool commandAvailable(environment *env)
{
	return (env->commandCounter < env->com->pointer) && (env->commandCounter >= 0);
}

int getInput(environment *env)
{
	if(env->input == NULL || (env->inputPtr >= env->inputSize))
	{
		return -1;
	}
	
	char retVal = env->input[env->inputPtr];
	env->inputPtr++;
	return retVal;
}

unsigned int runEnvironment(environment *env, unsigned int maxInstructions)
{	
	unsigned int instructions = 0;
	while(commandAvailable(env) && instructions < maxInstructions)
	{
		instructions++;
		char curCom = env->com->commands[env->commandCounter];
		
		if(curCom == '>')
		{
			env->arrayPtr++;
			
			if(env->arrayPtr >= ARRAY_SIZE)
			{
				env->arrayPtr = 0;
			}
		}
		else if(curCom == '<')
		{
			env->arrayPtr--;
						
			if(env->arrayPtr < 0)
			{
				env->arrayPtr = ARRAY_SIZE-1;
			}
		}
		else if(curCom == '+')
		{
			env->array[env->arrayPtr]++;
		}
		else if(curCom == '-')
		{
			env->array[env->arrayPtr]--;
		}
		else if(curCom == '.')
		{
			addOutput(env->output, env->array[env->arrayPtr]);
		}
		else if(curCom == ',')
		{
			// TODO: Implement this
			int input = getInput(env);
			if(input != -1)
			{
				env->array[env->arrayPtr] = input;
			}
		}
		else if(curCom == '[')
		{
			if(env->array[env->arrayPtr] == 0)
			{
				//Find closing bracket
				int bal = 1;
				do 
				{
					env->commandCounter++;
					if(env->com->commands[env->commandCounter] == '[') 
					{
						bal++;
					}
					else if (env->com->commands[env->commandCounter] == ']')
					{
						bal--;
					}
				}while ( bal != 0 && commandAvailable(env));
			}
		}
		else if(curCom == ']')
		{
			int bal = 0;
			do 
			{
				if(env->com->commands[env->commandCounter] == '[') 
				{
					bal++;
				}
				else if (env->com->commands[env->commandCounter] == ']') 
				{
					bal--;
				}
				env->commandCounter--;
			} while ( bal != 0 && commandAvailable(env));
		}
		
		env->commandCounter++;
	}
	return instructions;
}

void printArray(environment *env, int len)
{
	printf("Raw array: ");
	for(int i=0; i<len; i++)
	{
		if(i%16 == 0)
		{
			printf("\n");
		}
		else if(i%16 == 8)
		{
			printf(" ");
		}
		
		printf("0x%02X ", env->array[i]);
	}
	printf("\n");
}

void cleanupEnvironment(environment *env)
{
	cleanupCommand(env->com);
	cleanupOutput(env->output);

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

#define OP_MASK 0xE0
#define COUNT_MASK ~OP_MASK

#define OP_PTR_INC 		(0<<5)
#define OP_PTR_DEC 		(1<<5)
#define OP_DATA_INC 	(2<<5)
#define OP_DATA_DEC 	(3<<5)
#define OP_INPUT 		(4<<5)
#define OP_OUTPUT 		(5<<5)
#define OP_LOOP_START 	(6<<5)
#define OP_LOOP_STOP 	(7<<5)


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
	
	unsigned int instructions = runEnvironment(&env, (0xffffffff-1));
	printf("Instructions required: %u\n", instructions);
	printOutput(env.output);
	//printArray(&env, 32);
	
	cleanupEnvironment(&env);
}