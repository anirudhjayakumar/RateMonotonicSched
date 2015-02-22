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
