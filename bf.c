#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define INITIAL_INSTRUCTION_SIZE 1024
#define INITIAL_OUTPUT_SIZE 1024

#define ARRAY_SIZE 32768

#define OP_MASK (unsigned char)0xE0
#define COUNT_MASK (unsigned char)~OP_MASK

#define OP_PTR_INC 		(unsigned char)(0<<5)
#define OP_PTR_DEC 		(unsigned char)(1<<5)
#define OP_DATA_INC 	(unsigned char)(2<<5)
#define OP_DATA_DEC 	(unsigned char)(3<<5)
#define OP_INPUT 		(unsigned char)(4<<5)
#define OP_OUTPUT 		(unsigned char)(5<<5)
#define OP_LOOP_START 	(unsigned char)(6<<5)
#define OP_LOOP_STOP 	(unsigned char)(7<<5)

#define getOP(x) 		(x&OP_MASK)
#define isOP(x, y)		(getOP(x)==getOP(y))
#define isNOP(x)		(getCount(x) == 0)
#define getCount(x) 	(x&COUNT_MASK)

char printOperator(unsigned char c)
{
	switch(getOP(c))
	{
		case OP_PTR_INC: return '>';
		case OP_PTR_DEC: return '<';
		case OP_DATA_INC: return '+';
		case OP_DATA_DEC: return '-';
		case OP_OUTPUT: return '.';
		case OP_INPUT: return ',';
		case OP_LOOP_START: return '[';
		case OP_LOOP_STOP: return ']';
	}
	return '!';
}

typedef struct
{
	unsigned char *commands;
	int size;
	int reserved;
}commands;

void initCommand(commands *com)
{
	com->commands = NULL;
	com->size = 0;
	com->reserved = 0;
}

bool addCommand(commands *com, unsigned char c)
{
	if(com->size >= com->reserved)
	{
		// Re allocate memory
		int newBufferSize = com->reserved+INITIAL_INSTRUCTION_SIZE;
		
		com->commands = realloc(com->commands, newBufferSize);
		
		if(com->commands == NULL)
		{
			return false;
		}
		memset(com->commands+com->reserved, 0x00, INITIAL_INSTRUCTION_SIZE);
		com->reserved = newBufferSize;
	}
	
	unsigned char curCom = com->commands[com->size];
	printf("%c %c %s %d 0x%02x %s\n", printOperator(c), printOperator(curCom), isOP(curCom, c) ? "same" : "diff", getCount(curCom), COUNT_MASK, getCount(curCom) <= COUNT_MASK ? "small" : "large");
	
	if(isNOP(curCom))
	{
		com->commands[com->size] = c|0x01; // Current command is NOP only set the current command
	}
	else if(isOP(curCom, c) && getCount(curCom) <= COUNT_MASK) 
	{
		unsigned char count = getCount(curCom);
		printf("Same command: %c %d curCom: 0x%02x ", printOperator(c), count, curCom);

		count++;
		com->commands[com->size] = c|count;
		
		printf("newCom: %02x count: %d\n", com->commands[com->size], count);
	}
	else
	{
		com->size++;
		com->commands[com->size] = c|0x01;
	}
	return true;
}

void printCommand(commands *com)
{
	printf("Size: %d Commands BufferSize: %d\n", com->size, com->reserved);
	for(int i=0; i<com->size; i++)
	{
		unsigned char curCom = com->commands[i];
		printf("%c number %d\n", printOperator(curCom), getCount(curCom));
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
	return (env->commandCounter < env->com->size) && (env->commandCounter >= 0);
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

void incPtr(environment *env,unsigned char count)
{
	for(unsigned char i=0; i<count; i++)
	{
		env->arrayPtr++;
		
		if(env->arrayPtr >= ARRAY_SIZE)
		{
			env->arrayPtr = 0;
		}
	}
}

void decPtr(environment *env,unsigned char count)
{
	for(unsigned char i=0; i<count; i++)
	{
		env->arrayPtr--;
						
		if(env->arrayPtr < 0)
		{
			env->arrayPtr = ARRAY_SIZE-1;
		}
	}
}

void loopStart(environment *env,unsigned char count)
{
	for(unsigned char i=0; i<count; i++)
	{
		if(env->array[env->arrayPtr] == 0)
		{
			//Find closing bracket
			int bal = 1;
			do 
			{
				env->commandCounter++;
				if(isOP(env->com->commands[env->commandCounter], OP_LOOP_START)) 
				{
					bal++;
				}
				else if (isOP(env->com->commands[env->commandCounter], OP_LOOP_STOP))
				{
					bal--;
				}
			}while ( bal != 0 && commandAvailable(env));
		}
	}
}

void loopStop(environment *env,unsigned char count)
{
	for(unsigned char i=0; i<count; i++)
	{
		int bal = 0;
		do 
		{
			if(isOP(env->com->commands[env->commandCounter], OP_LOOP_START)) 
			{
				bal++;
			}
			else if (isOP(env->com->commands[env->commandCounter], OP_LOOP_STOP)) 
			{
				bal--;
			}
			env->commandCounter--;
		} while ( bal != 0 && commandAvailable(env));
	}
}

unsigned int runEnvironment(environment *env, unsigned int maxInstructions)
{	
	unsigned int instructions = 0;
	while(commandAvailable(env) && instructions < maxInstructions)
	{
		char curCom = env->com->commands[env->commandCounter];
		
		if(isNOP(curCom))
		{
			continue;	// Skip NOPs
		}
		
		instructions++;
		unsigned char count = getCount(curCom);
		
		if(isOP(curCom, OP_PTR_INC))
		{
			incPtr(env, count);
		}
		else if(isOP(curCom, OP_PTR_DEC))
		{
			decPtr(env, count);
		}
		else if(isOP(curCom, OP_DATA_INC))
		{
			env->array[env->arrayPtr] += count;
		}
		else if(isOP(curCom, OP_DATA_DEC))
		{
			env->array[env->arrayPtr] -= count;
		}
		else if(isOP(curCom, OP_OUTPUT))
		{
			for(unsigned char i=0; i<count; i++)
			{
				addOutput(env->output, env->array[env->arrayPtr]);
			}
		}
		else if(isOP(curCom, OP_INPUT))
		{
			for(unsigned char i=0; i<count; i++)
			{
				int input = getInput(env);
				if(input != -1)
				{
					env->array[env->arrayPtr] = input;
				}
			}
		}
		else if(isOP(curCom, OP_LOOP_START))
		{
			loopStart(env, count);
		}
		else if(isOP(curCom, OP_LOOP_STOP))
		{
			loopStop(env, count);
		}
		else
		{
			printf("NOP\n");
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
			addCommand(com, OP_PTR_INC);
			break;
			
			case '<':
			addCommand(com, OP_PTR_DEC);
			break;
			
			case '+':
			addCommand(com, OP_DATA_INC);
			break;
			
			case '-':
			addCommand(com, OP_DATA_DEC);
			break;
			
			case '.':
			addCommand(com, OP_OUTPUT);
			break;
			
			case ',':
			addCommand(com, OP_INPUT);
			break;
			
			case '[':
			addCommand(com, OP_LOOP_START);
			break;
			
			case ']':
			addCommand(com, OP_LOOP_STOP);
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
	
	printf("OP_PTR_INC 0x%02x\n", OP_PTR_INC);
	printf("OP_PTR_DEC 0x%02x\n", OP_PTR_DEC);
	printf("OP_DATA_INC 0x%02x\n", OP_DATA_INC);
	printf("OP_DATA_INC 0x%02x\n", OP_DATA_DEC);
	printf("OP_INPUT 0x%02x\n", OP_INPUT);
	printf("OP_OUTPUT 0x%02x\n", OP_OUTPUT);
	printf("OP_LOOP_START 0x%02x\n", OP_LOOP_START);
	printf("OP_LOOP_STOP 0x%02x\n", OP_LOOP_STOP);
	
	unsigned int p = OP_DATA_DEC|0x01;
	printf("p: 0x%02x, %c %s : %d %s\n", p, printOperator(p), isOP(p, OP_DATA_DEC) ? "true" : "false", getCount(p), (getCount(p)<COUNT_MASK) ? "smaller": "larger" );
	
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