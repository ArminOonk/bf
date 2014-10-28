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

void copyCommand(commands *from, commands *to)
{
	cleanupCommand(to);
	initCommand(to);
	for(int i=0; i<from->size; i++)
	{
		addCommand(to, from->commands[i]); 
	}
}

void cleanupCommand(commands *com)
{
	if(com->commands != NULL)
	{
		free(com->commands);
	}
}

bool commandAvailable(environment *env)
{
	return (env->commandCounter < env->com->size) && (env->commandCounter >= 0);
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

void cleanupOutput(output *out)
{
	if(out->buffer != NULL)
	{
		free(out->buffer);
	}
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

void cleanupEnvironment(environment *env)
{
	cleanupCommand(env->com);
	cleanupOutput(env->output);

	free(env->com);
	free(env->output);
}
