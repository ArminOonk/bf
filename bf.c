#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		return -1;
	}
	
	FILE *readFP = fopen(argv[1], "r");
	if ( readFP == NULL )
	{
		printf("Could not read input file: %s\n", argv[1]);
		return -1;
	}
	
	printf("Hello, World!\n");
}