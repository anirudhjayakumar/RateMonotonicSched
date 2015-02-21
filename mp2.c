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
