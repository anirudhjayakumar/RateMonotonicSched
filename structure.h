enum states {
	SLEEPING,	// 0
	READY,		// 1
	RUNNING,	// 2
}

typedef unsigned long ulong;
typedef struct list_head list_node;

ulong initial_jiffies, curr_jiffies;

/* Creating mutex lock to protect data structures when they are read or written */
struct mutex mymutex;

typedef struct process_entry {
	/* Data Structure elements */		/* Data Strcuture element explnation */
	pid_t pid;				// Process ID
	ulong period;				// Period of the process
	ulong computation;			// Computation time of process
	ulong c;
	enum state states;			// States of operation of the process coming from enum
	struct sched_param sparam;		// Scheduling parameter of the process
	struct timer_list mytimer;		// Timer for wakingup
	struct task_struct *task;		// Linux task pointer
	list_node mynode;
} my_process_entry;

/* Pointer to the currently running task */
my_process_entry *entry_currtask=NULL;

typedef struct proc_dir_entry procfs_entry;

procfs_entry *newproc = NULL;
procfs_entry *newdir = NULL;
procfs_entry *newentry = NULL;

procfs_entry* proc_filesys_entries(char *procname, char *parent);
static void remove_entry(char *procname, char *parent);
/* Admission Control Function Declaration */
int admission_control (my_process_entry *new_process_entry);
/* Remove task function declaration used when a process is deregistering */
int remove_task(pid_t pid);
/* mytimer call back interface */
void mytimer_callback(ulong data);
/* Worker thread to preempt the lower priority process and to execute the higher priority task */
int workthread(void *data);
