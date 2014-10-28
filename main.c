#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 
#include "bf.h"

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1);
int readFile(char *filename, commands *com);
void printOutput(output *out, int max);
void randomCommands(commands *com, int max);
void printCommand(commands *com);

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

#define ENV_LIST_SIZE 100
#define NR_ELITE 10

void newGeneration(environment envList[2][ENV_LIST_SIZE], int currentIndex)
{
	for(int i=0; i<ENV_LIST_SIZE; i++)
	{
		if(envList[currentIndex][i].com->size == 0)
		{
			randomCommands(envList[currentIndex][i].com, 256);
		}
	}
}

commands bestCommand;
void runGeneration(environment envList[2][ENV_LIST_SIZE], int currentIndex)
{
	commands eliteCommand[NR_ELITE];
	unsigned int fitness[ENV_LIST_SIZE];
	unsigned int instructions[ENV_LIST_SIZE];
	for(int i=0; i<NR_ELITE ; i++)
	{
		initCommand(&eliteCommand[i]);
	}
	
	for(int i=0; i<ENV_LIST_SIZE; i++)
	{
		instructions[i] = runEnvironment(&envList[currentIndex][i], 0xffff); // MAX: (0xffffffff-1)
		fitness[i] = fitnessFunction(&envList[currentIndex][i]);
	}
	
	for(int eliteIndex=0; eliteIndex<NR_ELITE; eliteIndex++)
	{
		unsigned int bestFitness = 0;
		int bestIndex = -1;

		for(int i=0; i<ENV_LIST_SIZE; i++)
		{
			if(fitness[i] > 0)
			{
				if(fitness[i] > bestFitness)
				{
					bestFitness = fitness[i];
					bestIndex = i;			
				}
				
				if(fitness[i] >= maxFitness())
				{
					printf("Solution found!\n");
					printCommand(envList[currentIndex][i].com);
					printOutput(envList[currentIndex][i].output, 10);
					return;				
				}
			}
		}
		
		if(bestIndex == -1)
		{
			break; // No more sufficient solutions
		}
		
		printf("Instructions required: %u Fitness: %u\n", instructions[bestIndex], fitness[bestIndex]);
		copyCommand(envList[currentIndex][bestIndex].com, &bestCommand);
		copyCommand(envList[currentIndex][bestIndex].com, &eliteCommand[eliteIndex]);
		fitness[bestIndex] = 0;
	}
				
	
	int nextIndex = 0;
	if(currentIndex == 0)
	{
		nextIndex = 1;
	}
	
	int eliteFound = 0;
	for(int i=0; i<NR_ELITE ; i++)
	{
		if(eliteCommand[i].size > 0)
		{
			copyCommand(&eliteCommand[i], envList[nextIndex][i].com);
			eliteFound++;
		}
		cleanupCommand(&eliteCommand[i]);
	}
	printf("Elite: %d\n", eliteFound);
}

void cleanupGeneration(environment envList[2][ENV_LIST_SIZE], int currentIndex)
{
	for(int i=0; i<ENV_LIST_SIZE; i++)
	{
		cleanupEnvironment(&envList[currentIndex][i]);
		initEnvironment(&envList[currentIndex][i]);
	}
}
	
int main(int argc, char **argv)
{
	int maxTries = 100;
	unsigned int bestFitness = 0;

	srand(time(NULL));
	
	struct timeval tStart, tEnd;
	
	
	initCommand(&bestCommand);
	
	environment envList[2][ENV_LIST_SIZE];
	int currentIndex = 0;

	for(int i=0; i<ENV_LIST_SIZE; i++)
	{
		initEnvironment(&envList[0][i]);
		initEnvironment(&envList[1][i]);
	}
	
	gettimeofday(&tStart, NULL);
	for(int i=0; i<maxTries; i++)
	{		
		newGeneration(envList, currentIndex);
		runGeneration(envList, currentIndex);
		cleanupGeneration(envList, currentIndex);
		printf("Round %d/%d: ", i+1, maxTries);
		
		if(currentIndex == 0)
		{
			currentIndex = 1;
		}
		else
		{
			currentIndex = 0;
		}
	}
	
	gettimeofday(&tEnd, NULL);
	struct timeval tDiff;
	timeval_subtract(&tDiff, &tEnd, &tStart);
    printf("Computation took: %ld.%06ld seconds\n", tDiff.tv_sec, tDiff.tv_usec);
	printf("Best fitness: %u\n", bestFitness);
	printf("Best commands\n");
	printCommand(&bestCommand);
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
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

void printCommand(commands *com)
{
	printf("Size: %d Commands BufferSize: %d\n", com->size, com->reserved);
	for(int i=0; i<com->size; i++)
	{
		printf("%c", com->commands[i]);
	}
	printf("\n");
}
