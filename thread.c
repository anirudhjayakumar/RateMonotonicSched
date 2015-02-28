#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/time.h>   // for using jiffies 
#include <linux/timer.h>
#include "structure.h"
#include "linklist.h"
//mutex lock for accessing the list

//the dispatching thread
static struct task_struct *disp_thread;

//the current running task
extern my_process_entry *running; //defined in mp2.c

/*worker thread*/
int thread_callback(void* data) {
	struct process_info *proc_iter = NULL;
	
	//to set the current task to interruptible
	set_current_state(TASK_INTERRUPTIBLE);

	//the call back function is executed till the thread is stopped
	while(!kthread_should_stop){
		printk(KERN_INFO "Inside thread worker");

	
		//take the first node as the one with highest priority
		my_process_entry *node = NULL;
        

        ll_find_high_priority_task(&node);
		//traverse the list to get process with highest priority and which is in running state
		/*list_for_each_entry(proc_iter,&proc_list.list,list){
			if(proc_iter->state == READY && proc_iter->pid != running->pid){	
				if(proc_iter->sparam.sched_priority > node->sparam.sched_priority){
					node = proc_iter;
				}	
			}
		}*/		

		//put the running task to ready state
		if(running->state != SLEEPING)
			running->state = READY;
		running->sparam.sched_priority = 0;
		sched_setscheduler(running->pid,SCHED_NORMAL,&(running->sparam));	

		//set the new task to running
		if( !node ) 
		{
			node->state = RUNNING;
			wake_up_process(node);
			node->sparam.sched_priority = 99;
			sched_setscheduler(node->pid,SCHED_FIFO,&(node->sparam));	//scheduling policy make a check
			running = node;
		}
	}	
	return 0;
}

//to initialize the thread
int thread_init (void) {
	printk(KERN_INFO "in init");
	
	//creates a dispatching thread
	disp_thread = kthread_create(thread_callback,NULL,"disp_thread");	
        return 0;
}

//to clean the dispatching thread
void thread_cleanup(void) {
	int ret;
	
	//to stop the dispatching thread
	ret = kthread_stop(disp_thread);
	if(!ret)
	printk(KERN_INFO "Thread stopped");
}

int wake_thread(void)
{
	wake_up_process(disp_thread);
	return SUCCESS; 
}
