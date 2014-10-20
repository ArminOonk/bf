#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define INITIAL_INSTRUCTION_SIZE 1024

typedef struct
{
	char *commands;
	int size;
	int bufferSize;
}commands;

bool addCommand(commands *com, char c)
{

	if(com->size >= com->bufferSize)
	{
		// Re allocate memory
		int newBufferSize = com->bufferSize+INITIAL_INSTRUCTION_SIZE;
		//printf("Allocating memory. Current size: %d New size: %d\n", com->bufferSize, newBufferSize);
		
		com->commands = realloc(com->commands, newBufferSize);
		if(com->commands == NULL)
		{
			return false;
		}
		com->bufferSize = newBufferSize;
	}
	com->commands[com->size] = c;
	com->size++;
	return true;
}

void cleanupCommand(commands *com)
{
	free(com->commands);
}

void initCommand(commands *com)
{
	com->commands = NULL;
	com->size = 0;
	com->bufferSize = 0;
}

void printCommand(commands *com)
{
	printf("Size: %d bufferSize: %d\n", com->size, com->bufferSize);
	for(int i=0; i<com->size; i++)
	{
		printf("%c", com->commands[i]);
	}
	printf("\n");
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
	
	commands com;
	initCommand(&com);
	
	if(readFile(argv[1], &com) < 0)
	{
		printf("Error reading file\n");
	}
	
	printCommand(&com);
	cleanupCommand(&com);
}