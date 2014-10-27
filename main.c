#include "bf.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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