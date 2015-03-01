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

//the current entry_curr_task task
extern my_process_entry *entry_curr_task; //defined in mp2.c

/*worker thread*/
int thread_callback(void* data) {
	struct process_info *proc_iter = NULL;
	printk(KERN_INFO "thread_callback second half\n");
	//to set the current task to interruptible
	//set_current_state(TASK_INTERRUPTIBLE);

	//the call back function is executed till the thread is stopped
	while(!kthread_should_stop()){
		printk(KERN_INFO "Inside thread worker");

	
		//take the first node as the one with highest priority
		my_process_entry *node = NULL;
        


		//put the entry_curr_task task to ready state
		if(entry_curr_task && entry_curr_task->state == RUNNING){
		
		    printk(KERN_INFO "workthread: update state of curr task\n");
			entry_curr_task->state = READY;
			entry_curr_task->sparam.sched_priority = 0;
			set_task_state(entry_curr_task->task,TASK_UNINTERRUPTIBLE);
			sched_setscheduler(entry_curr_task->pid,SCHED_NORMAL,&(entry_curr_task->sparam));
		}
        ll_find_high_priority_task(&node);
		//set the new task to entry_curr_task
		if( node ) 
		{
			printk(KERN_INFO "high priority task found with pid=%d",node->pid);
			node->state = RUNNING;
			wake_up_process(node);
			node->sparam.sched_priority = 99;
			sched_setscheduler(node->pid,SCHED_FIFO,&(node->sparam));	//scheduling policy make a check
			entry_curr_task = node;
		}
		
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
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
