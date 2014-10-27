#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bf.h"

void initCommand(commands *com)
{
	com->commands = NULL;
	com->size = 0;
	com->reserved = 0;
}

bool addCommand(commands *com, char c)
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
		com->reserved = newBufferSize;
	}
	com->commands[com->size] = c;
	com->size++;
	return true;
}

void printCommand(commands *com)
{
	printf("Size: %d Commands BufferSize: %d\n", com->size, com->reserved);
	for(int i=0; i<com->size; i++)
	{
		printf("%c", com->commands[i]);
	}
	printf("\n");
}

void cleanupCommand(commands *com)
{
	free(com->commands);
}

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

void printOutput(output *out, int max)
{
	if(max == -1 || max > out->length)
	{
		max = out->length;
	}
	
	
	printf("Output size %d : ", out->length);
	for(int i=0; i<max; i++)
	{
		printf("%c", out->buffer[i]);
	}
	printf("\n");
}

void cleanupOutput(output *out)
{
	free(out->buffer);
}

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


char randCommand()
{
	switch(rand()%9)
	{
		case 0: return '>';
		case 1: return '<';
		case 2: return '+';
		case 3: return '-';
		case 4: return '.';
		case 5: return ',';
		case 6: return '[';		
		case 7: return ']';
		default: return ' ';
	}
}

void randomCommands(commands *com, int max)
{
	for(int i=0; i<rand()%max; i++)
	{
		addCommand(com, randCommand());
	}
}
