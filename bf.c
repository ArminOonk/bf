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

void printOutput(output *out)
{
	printf("Output size %d : ", out->pointer);
	for(int i=0; i<out->pointer; i++)
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
	
	char array[ARRAY_SIZE];
	int pointer;
	
	int commandCounter;
}environment;

void initEnvironment(environment *env)
{
	env->pointer = 0;
	memset(env->array, 0x00, ARRAY_SIZE);
	
	env->commandCounter = 0;

	env->com = malloc(sizeof(commands));
	env->output = malloc(sizeof(output));

	initCommand(env->com);
	initOutput(env->output);
}

bool commandAvailable(environment *env)
{
	return (env->commandCounter < env->com->pointer) && (env->commandCounter >= 0);
}

void runEnvironment(environment *env)
{
	//bool finished = false;
	
	while(commandAvailable(env))
	{
		char curCom = env->com->commands[env->commandCounter];
		
		if(curCom == '>')
		{
			env->pointer++;
			
			if(env->pointer >= ARRAY_SIZE)
			{
				env->pointer = 0;
			}
		}
		else if(curCom == '<')
		{
			env->pointer--;
						
			if(env->pointer < 0)
			{
				env->pointer = ARRAY_SIZE-1;
			}
		}
		else if(curCom == '+')
		{
			env->array[env->pointer]++;
		}
		else if(curCom == '-')
		{
			env->array[env->pointer]--;
		}
		else if(curCom == '.')
		{
			addOutput(env->output, env->array[env->pointer]);
		}
		else if(curCom == ',')
		{
			// TODO: Implement this
		}
		else if(curCom == '[')
		{
			if(env->array[env->pointer] == 0)
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

		/*if(!commandAvailable(env))
		{
			finished = true;
		}*/
	}
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
	
	runEnvironment(&env);
	printOutput(env.output);
	printArray(&env, 32);
	
	cleanupEnvironment(&env);
}