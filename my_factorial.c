/****************************************/
/*	Author: Debjit Pal		*/
/*	NetID: dpal2			*/
/*	Group: 2			*/
/****************************************/

#include "my_factorial.h"

int main(int argc, char* argv[])
{
	int NumberRange;
	unsigned long int IntermediateResult;
	int iterator_1, iterator_2;
	FILE *fpt;
	int outer_index,o_o_index;
	/* If number range is not provided, can't proceed for factorial caluclation */
	if(argc != 2) {
		printf("You should provide range of number for Factorial.\n");
		return 0;
	}

	/* Check if /proc/mp1/status exists or not. If not issue a message and exit */ 
	fpt = fopen("/proc/mp1/status", "w");
	if(fpt == NULL) {
		printf("File /proc/mp1/status does not exist. Exiting..\n");
		return 0;
	}
	printf("PID of the current process: %d.\n", getpid());
	fprintf(fpt, "%d\n", getpid());
	fclose(fpt);

	/* Calculating factorial from 0 to the max number iteratively */	
	NumberRange = atoi(argv[1]);
	printf("Calculating Factorial Number Range: 0 - %d.\n", NumberRange);
	for (o_o_index = 0;o_o_index < 1000; ++o_o_index) {
	for (outer_index = 0;outer_index < 100000; ++outer_index) {
		for(iterator_1 = 0; iterator_1 <= NumberRange; iterator_1++) {
			if(iterator_1 == 0) {
//				printf("Calculated Value of 0! = 1.\n");
				continue;
			}
			else {
				IntermediateResult = 1;
				for(iterator_2 = iterator_1; iterator_2 > 0; iterator_2--) {
					IntermediateResult *= iterator_2;
				}
//				printf("Calculated Value of %d! = %ld.\n", iterator_1, IntermediateResult);
			}
		}
	}
	}
    	system("cat /proc/mp1/status");
	return 0;
}
