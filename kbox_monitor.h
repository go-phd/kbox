
#ifndef _KBOX_MONITOR_H
#define _KBOX_MONITOR_H

#include <linux/sched.h>

#include "kbox.h"

#define KBOX_SLEEP_TIME	1000

struct kbox_monitor_s {
	struct task_struct *monitor_thread;
};


int kbox_init_monitor(void);
void kbox_cleanup_monitor(void);


#endif

