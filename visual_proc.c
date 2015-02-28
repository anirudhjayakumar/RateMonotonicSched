#include <stdio.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <errno.h>
#include <sys/time.h>

void myregister(pid_t pid, uint32_t period, uint32_t computation, char *file)
{
	FILE *fp=fopen(file,"w");
	int ret;

	printf("R,pid=%lu,period=%lu,compu=%lu\n",pid,period,computation);
	ret=fprintf(fp,"R,%lu,%lu,%lu",pid,period,computation);
	fclose(fp);
}

int read_status(char *file)
{
	int ret, filenum, i;
	unsigned long pid, period, comp;	// dummy variables
	pid_t mypid = getpid();
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

	printf("address of buffer is %p\n",buffer);
	printf("the read is: %s\n", buffer);
	printf("numbytes is: %d\n", ret);

	//check if the process is in the list, if so return ret, else return 0
	while(*checker != '\0' && (checker - buffer < sizeof(buffer)))
	{
		sscanf(checker, "%6lu,%6lu,%6lu\n", &pid, &period, &comp);
		
		printf("CHECK: %lu, %lu, %lu\n", pid, period, comp);
		
		if(pid == mypid)
			return ret;
	
		checker += 21;
	}

	printf("REGISTRATION DENIED\n");	
	return 0;
}

unsigned long factorial(unsigned long i)
{
	unsigned long j,fac=1;
	for(j=2;j<=i;j++)
		fac*=j;
	return fac;
}

void visible_test_sleep(unsigned long i)
{
	unsigned long count;

	for(count = 0; count < i; count++)
		printf("%ld\n", count);
}

void yield(pid_t pid,char *file)
{
	FILE *fp=fopen(file,"w");
	fprintf(fp,"Y,%lu",pid);
	fclose(fp);
}

void unregister(pid_t pid, char *file)
{
	FILE *fp=fopen(file,"w");
	fprintf(fp,"D,%lu",pid);
	fclose(fp);
}


int main()
{

	char *file="/proc/mp2/status";
	uint32_t period=15000, computation=1000;
	pid_t pid;
	int reg_success;
	unsigned long i, jobs = 2;
	unsigned int ret;
	char buf[1000];
	struct timeval t0, current;

	pid=getpid();
	myregister(pid,period,computation, file);

	reg_success=read_status(file);
	
	if(!reg_success)
		exit(1);

	//get the time of day
	gettimeofday(&t0, NULL);

	yield(pid,file);
	for(i=0;i<jobs;i++){

		printf("\npid %d running %lu times\n",pid,i);
		//get the time of day
		gettimeofday(&current, NULL);
		printf("currently running for: %ld seconds, %ld miliseconds\n", 
			current.tv_sec - t0.tv_sec, 
			((current.tv_usec%1000000) - (t0.tv_usec%1000000))/1000);

		//factorial(100000000000);
		visible_test_sleep(10000000);
		yield(pid,file);
	}

	printf("\npid %d running %lu times\n",pid,i);
	//get the time of day
	gettimeofday(&current, NULL);
	printf("currently running for: %ld seconds, %ld miliseconds\n", 
		current.tv_sec - t0.tv_sec, 
		((current.tv_usec%1000000) - (t0.tv_usec%1000000))/1000);
	
	printf("done2");
	unregister(pid,file);
	return 0;

}
