//////////////////////////////////////////////////
//	Author: Debjit Pal			//
//	Email: dpal2@illinois.edu		//
//////////////////////////////////////////////////

#include <linux/module.h>	/* Specifically for a module */
#include <linux/kernel.h>	/* We are doing kernel work after all */
#include <linux/timer.h>	/* For working with the timer */
#include <linux/proc_fs.h>	/* Necessary because we use proc file system*/
#include <linux/list.h>		/* Necessary because we will be using kernel linked list */
#include <asm/uaccess.h>	/* Necessary for the funtion copy_from_user and copy_to_user */
#include <linux/sched.h>	/* Necessary to get the access of linux kernel scheduler APIs 
				   And to get the pointer of type struct task_struct */
#include <linux/kthread.h>	
#include <linux/mutex.h>	/* Useful for accessing mutex lock APIs */
#include <linux/sysfs.h>	
#include <linux/jiffies.h>	/* Useful for jiffies counting */
#include <linux/types.h>	/* Useful to get the ssize_t, size_t, loff_t */
#include <linux/slab.h>		/* Useful for memory allocation using SLAB APIs */

#include "mp2_given.h"
#includd "structure.h"		/* Defining the state enum and the process control block structures */


/* 	Admission control makes sure that the new process which is trying to register can be accmodated
	given the present utilization of the CPU. If the new utilization is less than ln 2 or 0.693, 
	the new process gets admitted else its registration is denied.
*/

int admission_control (my_process_entry *new_process_entry) {
	ulong utilization = 0;
	my_process_entry *entry_temp;
	struct list_head *it, *next;
	/* Since floating point calculation is costly, we multiply by 1000 to make it an integer */
	utilization = (new_process_entry->computation)*1000 / (new_process_entry->period);

	mutex_lock(mymutex);
	/* Now go through the whole linked list and sum up the utilization ratio of the current
	   registered processes and the new process */
	list_for_each_safe(it, next, &mylist) {
		entry_temp = list_entry(it, my_process_entry, mynode);
		utilization = utilization + (entry_temp->computation)*1000/ (entry_temp->period);
	}
	mutex_unlock(&mymutex);
	printk(KERN_INFO "Utilization Sum: %lu", utilization);
	if(utilization <= 693) {
		return 0;
	}
	else {
		return -1;
	}
}


o
