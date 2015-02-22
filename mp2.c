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

static ssize_t procfile_write(struct file *file, const char __user *buffer, size_t count, loff_t *data) {
	
	pid_t pid;
	char *proc_buffer;
	char *pid_str = NULL, *period_str = NULL, *computation_str = NULL, *end;
	new_process_entry *entry_temp;
	ulong ret = 0;

	proc_buffer = (char *)kmalloc(count, GFP_KERNEL);
	if(copt_from_user(proc_buffer, bufferm count)) {
		kfree(proc_buffer);
		return -EFAULT;
	}
	proc_buffer[count] = '\0';
	/* 	Handle differnt cases of the process through the proc file system
		R : Case for new process trying to register
		Y : Case for a process yielding
		D : Case for a process done with computing and going to deregister
	*/
	switch(proc_buffer[0]) {
		case 'R':

			/* Changing all commas to null terminated strings and storing them. */
			proc_buffer[count] = '\0';
			pid_str = procfs_buffer + 2;
			end = strstr(procfs_buffer + 2, ",");
			*end = '\0';
			period_str = end + 1;
			end = strstr(procfs_buffer + 2, ",");
			*end = '\0';
			computation_str = end + 1;
			
			/* Creating a temporary entry in kernel space to hold the new requesting process */
			entry_temp = (my_process_entry *)kmalloc(sizeof(my_process_entry), GFP_KERNEL);
			/*
				kstrtoul : Convert a string to an unsigned long
				int kstrtoul ( 	const char *s,
						unsigned int base,
						unsigned long *res);
				s: The start of the string and it must be null terminated.
				base:  The number base to use
				res: Where to write the result after conversion is over
			*/
			if((ret = kstrtoul(pid_str, 10, &(entry_temp->pid))) == -1) { 
				printk(KERN_ALERT "ERROR IN PID TO STRING CONVERSION\n");
				kfree(proc_buffer);
				kfree(entry_temp);
				return -EFAULT;
			}
			if((ret = kstrtoul(period_str, 10, &(entry_temp->period))) == -1) {
				PRINTK(KERN_ALERT "ERROR IN PERIOD TO STRING CONVERSION\n");
				kfree(proc_buffer);
				kfree(entry_temp);
				return -EFAULT;
			}
			if((ret - kstrtoul(computation_str, 10, &(entry_temp->computation))) == -1) {
				printk(KERN_ALERT "ERROR IN COMPUTATION TO STRING CONVERSION\n");
				kfree(proc_buffer);
				kfree(entry_temp);
				return -EFAULT;
			}
		
			/* If every conversion success print the details in the kernel log for debugging purpose */
			printk(KERN_INFO "Registering Process PID = %lu, PERIOD = %lu, COMPUTATION = %lu\n", entry_temp->pid, entry_temp->period, entry_temp->computation);
		
			/* Call Admission Control function now to see if the new process can be registered */
			if(admission_control(entry_temp) == -1) {
				printk(KERN_ALRERT "DENIED REGISTRATION OF PID = %lu", entry_temp->pid);
				kfree(entry_temp);
				kfree(proc_buffer);
				return -EFAULT;
			}
		
			/* If we can register the process, then we have to check the time of registration */
			curr_jiffies = jiffies;
			printk(KERN_INFO "PROCESS PID = %lu REGISTERED: %lu\n", entry_temp->pid, jiffies_to_usecs(curr_jiffies));

			/* We directly dont modify the process control block or PCB of the newly admitted process rather we keep a pointer 
			   to the PCB of the newly admitted process as suggested in the MP doc. We use the find_task_by_pid() function provided
			   in th emp2_given.h file for this purpose.
			*/
			entry_temp->task = find_task_by_pid(entry_temp->pid);
			/* Once the pointer to PCB found, initialize the timer associated with the process 
			   See: http://www.ibm.com/developerworks/library/l-timers-list/
			   for setup_timer details
			*/
			setup_timer(&(entry_temp->mytimer), &mytimer_callback, (ulong)entry_temp);
			/* With an initialized timer, the user now needs to set the expiration time, which is done through a 
			   call to mod_timer. As users commonly provide an expiration in the future, 
			   they typically add jiffies here to offset from the current time.
			   See: http://www.ibm.com/developerworks/library/l-timers-list/
			   for mod_timer_details
			*/
			ret = mod_timer(&(entry_temp->mytimer), jiffies + msecs_to_jiffies(entry_temp->period - entry_temp->computation));
			/* Finally, users can determine whether the timer is pending (yet to fire) through a 
			   call to timer_pending (1 is returned if the timer is pending):
			   See: http://www.ibm.com/developerworks/library/l-timers-list/
			   for timer_pending details
			*/
			if(ret) {
				printk(KERN_ALERT "ERROR IN SET TIMER\n");
				printk(KERN_ALERT "TIMER PENDING IS: %lu\n", timer_pending(&(entry_temp->mytimer)));
			}
			entry_temp->state = SLEEPING; /* Set the process state to SLEEP and then add it to the process list */
			mutex_lock(&mymutex);
			list_add_tail(&(entry_temp->mynode), &mylist);
			mutex_unlock(&mymutex);

			break;
		case 'Y':
			/* Check if list is empty before yielding */
			if(list_empty(&mylist)) {
				printk(KERN_ALERT "PROCESS LIST IS EMPTY\n");
				kfree(proc_buffer);
				return -EFAULT;
			}



	} /* End of the switch statement*/


}





/* Similar procfile_read function like MP1 */

static ssize_t procfile_read (struct file *file, char __user *buffer, size_t count, loff_t *data) {
	char *read_buf = NULL;
	unsigned int buf_size;
	ulong ret = 0;
	ssize_t len = count, retVal = 0;

	printk(KERN_INFO "PROCFILE READ /proc/mp2/status CALLED\n");
	/* I am assuming Aniruddh will provide the buf size again */
	printk(KERN_INFO "*data = %d, buf_size = %d, count = %ld", (int)(*data), buf_size, count);
	if(*data >= buf_size) {
		kfree(read_buf);
		goto out;
	}

	if((int)(*data) + count > buf_size) {
		len = buf_size - (int)(*data);
	}


	if((ret = copy_to_user(buffer, read_buf,buf_size) != 0)) {
		printk(KERN_INFO "copy to user failed\n");
		return -EFAULT;
	}
	*data += (loff_t)(len - ret);
	retVal = len - ret;
	kfree(read_buf);
	out:
		return retVal;
}

static struct file_operations proc_file_op = {
	.owner 	= THIS_MODULE,
	.read	= procfile_read,
	.write	= procfile_write,
};

procfs_entry* proc_filesys_entries(char *procname, char *parent) {

	newdir = proc_mkdir(parent, NULL);
	if(newdir == NULL) {
		printk(KERN_ALERT "ERROR IN DIRECTORY CREATION\n");
	}
	else
		printk(KERN_ALERT "DIRECTORY CREATION SUCCESSFUL");
	
	newproc = proc_create(procname, 0666, newdir, &proc_file_op);
	if(newproc == NULL)
		printk(KERN_ALERT "ERROR: COULD NOT INITIALIZE /proc/%s/%s\n", parent, procname);

	printk(KERN_INFO "INFO: SUCCESSFULLY INITIALIZED /proc/%s/%s\n", parent, procname);
	
	return newproc;

}

static void remove_entry(char *procname, char *parent) {
	remove_proc_entry(procname, newdir);
	remove_proc_entry(parent, NULL);
}



static int __mp2_init(void) {
	printk("MP2 MODULE LOADING");
	printk("MODULE INIT CALLED");
	newentry = proc_filesystem_entries("status", "MP2");

	printk("MP2 MODULE LOADED");
	return 0;
}

static void __exit mp1_exit(void) {
	printk("MP2 MODULE UNLOADING");
	remove_entry("status", "mp2");
	printk("MP2 MODULE UNLOADED");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group_2");
MODULE_DESCRIPTION("CS-423_MP2");
