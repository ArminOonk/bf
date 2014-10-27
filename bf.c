#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

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

const char desiredResult[] = "HI";

unsigned int maxFitness()
{
	return 256*strlen(desiredResult);
}

unsigned int fitnessFunction(environment *env)
{
	unsigned int fitness = 0;
	int desiredLength = strlen(desiredResult);
	int end = env->output->length;
	
	if(end > desiredLength)
	{
		end = desiredLength;
	}
	
	for(int i=0; i<end; i++)
	{
		fitness += (256 - abs((int)env->output->buffer[i] - (int)desiredResult[i]));
	}
	
	return fitness;
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

int main(int argc, char **argv)
{			
	/*if(argc >= 2)
	{
		if(readFile(argv[1], env.com) < 0)
		{
			printf("Error reading file\n");
		}
	}
	else
	{
		randomCommands(env.com);
	}*/
	environment env;
	int maxTries = 1000000;
	unsigned int bestFitness = 0;
	unsigned int endFitness = maxFitness(); 
	srand(time(NULL));
	
	struct timeval tStart, tEnd;
	gettimeofday(&tStart, NULL);
	
	for(int i=0; i<maxTries; i++)
	{
		initEnvironment(&env);
		if(argc >= 2 && i==0)
		{
			if(readFile(argv[1], env.com) < 0)
			{
				printf("Error reading file\n");
			}
		}
		else
		{
			randomCommands(env.com, 256);
		}
		//randomCommands(env.com);
				
		unsigned int instructions = runEnvironment(&env, 0xffff); // MAX: (0xffffffff-1)
		unsigned int fitness = fitnessFunction(&env);
		
		if(fitness > 0)
		{
			if(fitness > bestFitness)
			{
				printf("Round %d/%d: ", i+1, maxTries);
				printf("Instructions required: %u Fitness: %u\n", instructions, fitness);
				bestFitness = fitness;
			}
			
			if(fitness >= endFitness)
			{
				printf("Solution found!\n");
				printCommand(env.com);
				printOutput(env.output, 10);
				break;				
			}
		}
				
		cleanupEnvironment(&env);
	}
	gettimeofday(&tEnd, NULL);
	struct timeval tDiff;
	timeval_subtract(&tDiff, &tEnd, &tStart);
    printf("Computation took: %ld.%06ld seconds\n", tDiff.tv_sec, tDiff.tv_usec);
	printf("Best fitness: %u\n", bestFitness);
}