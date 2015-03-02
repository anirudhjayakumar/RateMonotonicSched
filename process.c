#include <stdio.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <errno.h>
#include <sys/time.h>
#include <inttypes.h>
#include <string.h>

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
	pid_t mypid = getpid();
	char pid_str[10];
	char buffer[512];
	int ret;
	bzero(buffer, 512);

	FILE *fp = fopen(file, "r");

	if(fp == NULL)
		return -1;
		
	/*since the buffer returns nicely, make the
	read function return the number of process
	in the scheduler so we know what to parse*/
	ret = fread(buffer,sizeof(char),500,fp);
	fclose(fp);

	printf("Address of read buffer is %p\n",buffer);
	printf("Num of bytes read from buffer is: %d\n", ret);
	printf("PID from getpid() is: %d\n", mypid);
	
	sprintf(pid_str, "%d", (int)mypid);
	/*check if the process is in the list, if so return ret, else return 0*/
	if(strstr(buffer, pid_str) != NULL) {
		printf("PID found n /proc/MP2/status %s\n and hence this process is registered.", pid_str);
		return ret;
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
	uint32_t period=0, computation=0;
	pid_t pid;
	int reg_success;
	unsigned long jobs;
	int i;
	unsigned int ret;
	char buf[1000];
	struct timeval t0, start, end;

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
	/* process is trying to register */
	myregister(pid,period,computation,file);
        printf("Checking if registration is successful!!\n");
	reg_success=read_status(file);
	gettimeofday(&t0, NULL);
	if(!reg_success){
		exit(1);
	}
	else {
		printf("Registration done successful. PID found in status file at %ld seconds. \n", t0.tv_sec);
	}
    
	/* get the time of day */
	
	yield(pid,file);
	for(i=0;i<jobs;i++){

		printf("\npid %d running %d times\n",pid,i);
		/* get the time of starting the job*/
		gettimeofday(&start, NULL);
		factorial(20000);
		/* get the time of ending the job*/
		gettimeofday(&end, NULL);
		printf("Job %d started at: %ld useconds and finished at %ld useconds\n", i, start.tv_usec, end.tv_usec);
		yield(pid,file);
	}

	/* Did the following code when I was checking with only 1 job */
	/* printf("\npid %d running %lu times\n",pid,i);
	get the time of day
	gettimeofday(&current, NULL);
	printf("currently running for: %ld seconds, %ld miliseconds\n", 
		current.tv_sec - t0.tv_sec, 
		((current.tv_usec%1000000) - (t0.tv_usec%1000000))/1000); */
	
	printf("Done with Job.. Going to unregister..\n");
	unregister(pid,file);
	return 0;
}
