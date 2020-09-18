
#include <linux/kthread.h>
#include <linux/delay.h>

#include "kbox_dump.h"
#include "kbox_console.h"
#include "kbox_monitor.h"
#include "kbox_netlink.h"


struct kbox_monitor_s monitor;

static void monitor_cpu(struct kbox_monitor_s *monitor)
{
	int ret = 0;
	char *kmsg1 = "hello users1!!!";
	char *kmsg2 = "hello users2!!!";

	// debug
	return;

	ret = kbox_broadcast(KBOX_NLGRP_DEVICE_EVENT, KBOX_NL_CMD_REBOOT, kmsg1, strlen(kmsg1), GFP_ATOMIC);
	KBOX_LOG(KLOG_ERROR, "---kbox_broadcast KBOX_NLGRP_DEVICE_EVENT %s, ret = %d\n", kmsg1, ret);

	ret = kbox_broadcast(KBOX_NLGRP_SYSTEM_EVENT, KBOX_NL_CMD_PANIC, kmsg2, strlen(kmsg2), GFP_ATOMIC);
	KBOX_LOG(KLOG_ERROR, "---kbox_broadcast KBOX_NLGRP_SYSTEM_EVENT %s, ret = %d\n", kmsg2, ret);
}


static int monitor_thread(void *arg)
{
	struct kbox_monitor_s *monitor = (struct kbox_monitor_s *)arg;

	KBOX_LOG(KLOG_DEBUG, "monitor thread start\n");

	kbox_dump_event(KBOX_DIE_EVENT, 1, "test panic");
	kbox_dump_debug_print();

	while (!kthread_should_stop()) {
		//KBOX_LOG(KLOG_DEBUG, "\n");
		//kbox_console_debug_print();
		monitor_cpu(monitor);
		msleep(KBOX_SLEEP_TIME);
	}

	KBOX_LOG(KLOG_DEBUG, "monitor thread exiting\n");

	return 0;
}


int kbox_init_monitor(void)
{
	monitor.monitor_thread = kthread_run(monitor_thread, (void *)&monitor, "kbox monitor");
	if (IS_ERR(monitor.monitor_thread)) {
		KBOX_LOG(KLOG_ERROR, "kernel_run  edma_host_msg failed\n");
		return PTR_ERR(monitor.monitor_thread);
	}

	return 0;
}


void kbox_cleanup_monitor(void)
{
	kthread_stop(monitor.monitor_thread);
}



