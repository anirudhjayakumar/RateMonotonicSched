//this is the factorial program
#include <stdlib.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/types.h>


//a very naive factorial program
//we did not allocate a large enough value so
//after 10! this will most likely return 0
unsigned long factorial(unsigned long toFactorial)
{
	unsigned long factorial = 1, i;

	//loop through teh factorials and multiply i * i-1 if
	//not the base case
	for(i = 0; i < toFactorial; i++)
	{
		if(i == 0 || i == 1)
			factorial *= 1;
		else
			factorial *= i;
	}

	return factorial;	
}

//the main will just register itself and then call factorial
//it will return whatever factorial will call and if we 
//want to block most likely it will return zero
int main(int argc, char* argv[])
{
	//open the file and register
	unsigned long factorialed = 0;
	FILE* file = fopen("/proc/mp1/status", "w");

	printf("1\n");
	//write to teh file and call the factorial
	if(argc == 2 && file != NULL)	
	{
		fprintf(file, "%lu", getpid());
		fclose(file);	//close to ensure we write		
		printf("pid is: %lu\n", getpid()); //inform the user of the pid
		factorialed = factorial(atol(argv[argc-1]));
		printf("%lu\n", factorialed);
	}

	return 0;
}
