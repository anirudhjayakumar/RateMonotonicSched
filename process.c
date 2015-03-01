#include <stdio.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <errno.h>
#include <sys/time.h>
#include <inttypes.h>

/* When the process itself is trying to register it should call myregister along with the R to make sure kernel 
module gets the registration info */ 

void myregister(pid_t pid, unsigned long int period, unsigned long int computation, char *file) {
	FILE *fp=fopen(file,"w");
	int ret;
	/* using the key charecter 'R' for registration */
	printf("R,pid=%d,period=%lu,compu=%lu\n",pid,period,computation);
	ret=fprintf(fp,"R,%d,%lu,%lu",pid,period,computation);
	fclose(fp);
}

/* The application running in the system should be able to query which applications are registered ans also
 * query the scheduling parameters of each of the registered application. When the entry /proc/mp2/status 
 * is read by an application, the kernel module must return a list with the PID, Period and Processing 
 * Time of each application. This requirement is done by the following function read_status.
 * INPUT: takes the location of the status file
 * OUTPUT: outputs the parameters of the registered applications
 */

int read_status(char *file) {
	int ret, filenum, i;
	unsigned long period, comp;	// dummy variables
	pid_t mypid = getpid();
	pid_t pid;
	char buffer[512];
	char* checker = buffer;
	bzero(buffer, 512);

	FILE *fp = fopen(file, "r");

	if(fp == NULL)
		return -1;
		
	//since the buffer returns nicely, make the
	//read function return the number of process
	//in the scheduler so we know what to parse
	ret = fread(buffer,sizeof(char),500,fp);
	fclose(fp);

	printf("Address of buffer is %p\n",buffer);
	printf("The read is: %s\n", buffer);
	printf("Num bytes is: %d\n", ret);

	//check if the process is in the list, if so return ret, else return 0
	while(*checker != '\0' && (checker - buffer < sizeof(buffer))) {
		sscanf(checker, "%6d,%6lu,%6lu\n", &pid, &period, &comp);
		printf("CHECK: %d, %lu, %lu\n", pid, period, comp);
		
		if(pid == mypid)
			return ret;
	
		//size of each entry will be 28
		checker += 21;	
	}

	printf("REGISTRATION DENIED\n");	
	return 0;
}

/* Factorial calculation function. Every time we are calculating factorial of the same value */

unsigned long factorial(unsigned long i) {
	unsigned long j,fac=1;
	for(j=2;j<=i;j++)
		fac*=j;
	return fac;
}

void visible_test_sleep(unsigned long i) {
	unsigned long count;
	for(count = 0; count < i; count++)
		printf("%ld\n", count);
}

/* When a process wants to yield, it will call this function */

void yield(pid_t pid,char *file) {
	FILE *fp=fopen(file,"w");
	/* Use the key charecter 'Y' for yield */
	fprintf(fp,"Y,%d",pid);
	fclose(fp);
}

/* When a process is going to deregister, it will call this function */

void unregister(pid_t pid, char *file) {
	FILE *fp=fopen(file,"w");
	/* Use the key charecter 'D' for deregistration */
	fprintf(fp,"D,%d",pid);
	fclose(fp);
}

/* The main function that shall be called when the application will be invoked */

int main(int argc, char* argv[])
{

	char *file="/proc/MP2/status";
	uint32_t period=5000, computation=10;
	pid_t pid;
	int reg_success;
	unsigned long i, jobs;
	unsigned int ret;
	char buf[1000];
	struct timeval t0, current;

	printf("The number of arguments: %d\n", argc);

	if(argc != 4) {
		printf("Usage: ./process <period> <computation> <number of jobs>\n");
		printf("usage: Period and Computation time is in miliseconds\n");
		exit(0);
	}

	period = (unsigned long int) atol(argv[1]);
	computation = (unsigned long int) atol(argv[2]);
	jobs = (unsigned long int) atol(argv[3]);

	pid=getpid();
	myregister(pid,period,computation,file);
        printf("Registration done!!\n");
	reg_success=read_status(file);
 	printf("reg_success:%d\n",reg_success);	
	if(!reg_success)
		exit(1);
    
	/* get the time of day */
	gettimeofday(&t0, NULL);

	yield(pid,file);
	for(i=0;i<jobs;i++){

		printf("\npid %d running %lu times\n",pid,i);
		/* get the time of day
		yield(pid,file); */
		gettimeofday(&current, NULL);
		printf("currently running for: %ld seconds, %ld miliseconds\n", 
			current.tv_sec - t0.tv_sec, 
			((current.tv_usec%1000000) - (t0.tv_usec%1000000))/1000);

		printf("Calculated Factorial: %lu\n", factorial(2000));
		/* visible_test_sleep(1000000); */
		yield(pid,file);
	}

	/* Did the following code when I was checking with only 1 job */
	/* printf("\npid %d running %lu times\n",pid,i);
	get the time of day
	gettimeofday(&current, NULL);
	printf("currently running for: %ld seconds, %ld miliseconds\n", 
		current.tv_sec - t0.tv_sec, 
		((current.tv_usec%1000000) - (t0.tv_usec%1000000))/1000); */
	
	printf("Donr with Job.. Going to unregister..\n");
	unregister(pid,file);
	return 0;
}
