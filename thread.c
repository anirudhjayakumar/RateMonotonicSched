#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/time.h>   // for using jiffies 
#include <linux/timer.h>
#include<structure.h>
#include<linux/mutex.h>

//mutex lock for accessing the list
struct mutex lock;

//the dispatching thread
static struct task_struct *thread;

//the current running task
struct my_process_entry *running;	

/*worker thread*/
int thread_callback(void* data) {
	struct list_head *current,*next;
	struct process_info *proc_iter = NULL;

	//to set the current task to interruptible
	set_current_state(TASL_INTERRUPTIBLE);

	//the call back function is executed till the thread is stopped
	while(!kthread_should_stop){
		printk(KERN_INFO "Inside thread worker");

		mutex_lock(&lock);	//acquiring the lock
	
		//take the first node as the one with highest priority
		struct my_process_entry *node;
		node = getFirstNode();	//to be implemented

		//traverse the list to get process with highest priority and which is in running state
		list_for_each_entry(proc_iter,&proc_list.list,list){
			if(proc_iter->state == READY && proc_iter->pid != running->pid){	
				if(proc_iter->sparam.sched_priority > node->sparam.sched_priority){
					node = proc_iter;
				}	
			}
		}		
		mutex_unlock(&lock);	//releasing the lock

		//put the running task to ready state
		if(running->state != SLEEPING)
			running->state = READY;
		running->sparam.sched_priority = 0;
		sched_setscheduler(running->pid,SCHED_NORMAL,&(running->sparam))	

		//set the new task to running
		node->state = RUNNING;
		wake_up_process(node);
		node->sparam.sched_priority = 99;
		sched_setscheduler(node->pid,SCHED_FIFO,&(node->sparam))	//scheduling policy make a check
	}	
	return 0;
}

//to initialize the thread
int thread_init (void) {
	printk(KERN_INFO "in init");
	
	//creates a dispatching thread
	thread = kthread_create(thread_callback,NULL,"thread");	
        return 0;
}

//to clean the dispatching thread
void thread_cleanup(void) {
	int ret;
	
	//to stop the dispatching thread
	ret = kthread_stop(thread);
	if(!ret)
	printk(KERN_INFO "Thread stopped");
}

