enum states {
	SLEEPING,	// 0
	READY,		// 1
	RUNNING,	// 2
}

typedef unsigned long ulong;
typedef struct list_head list_node;

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

