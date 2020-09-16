#include <linux/kernel.h>
#include <linux/kdebug.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <asm/mce.h>

#include "kbox_notifier.h"
#include "kbox_collect.h"
#include "kbox_console.h"
#include "kbox_output.h"
#include "kbox_netlink.h"



static int kbox_reboot_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	int ret = 0;
	char str_buf[512] = {};
	int count = 0;
	
	count += sprintf(str_buf, "======reboot event start======\n");
	count += sprintf(str_buf + count, "---do some thing---\n");
	count += sprintf(str_buf + count, "======reboot event end======\n");
	KBOX_LOG(KLOG_DEBUG, "event = 0x%lx\n", event);

	ret = kbox_broadcast(KBOX_NLGRP_SYSTEM_EVENT, KBOX_NL_CMD_REBOOT, str_buf, strlen(str_buf), GFP_ATOMIC);
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_broadcast fail, ret = %d\n", ret););
	
	
	return NOTIFY_OK;
}


static struct notifier_block kbox_reboot_block = {
	.notifier_call	= kbox_reboot_event,
	.priority	= INT_MAX,
};


static int kbox_die_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	int ret = 0;
	char str_buf[1024] = {};
	int count = 0;

	
	count += sprintf(str_buf, "=die event=\n");
	count += sprintf(str_buf + count, "---do some thing---\n");
	count += sprintf(str_buf + count, "======die event end======\n");
	
	KBOX_LOG(KLOG_ERROR, "event = 0x%lx\n", event);
	ret = kbox_broadcast(KBOX_NLGRP_SYSTEM_EVENT, KBOX_NL_CMD_DIE, str_buf, strlen(str_buf), GFP_ATOMIC);
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_broadcast fail, ret = %d\n", ret););
	
	return NOTIFY_OK;
}


static struct notifier_block kbox_die_block = {
	.notifier_call	= kbox_die_event,
	.priority	= INT_MAX,
};


static int kbox_panic_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	int ret = 0;
	char str_buf[512] = {};
	int count = 0;

	
	count += sprintf(str_buf, "======panic event start======\n");
	count += sprintf(str_buf + count, "---do some thing---\n");
	count += sprintf(str_buf + count, "======panic event end======\n");

	KBOX_LOG(KLOG_ERROR, "event = 0x%lx\n", event);
	ret = kbox_broadcast(KBOX_NLGRP_SYSTEM_EVENT, KBOX_NL_CMD_PANIC, str_buf, strlen(str_buf), GFP_ATOMIC);
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_broadcast fail, ret = %d\n", ret););
	
	return NOTIFY_DONE;
}


static struct notifier_block kbox_panic_block = {
	.notifier_call	= kbox_panic_event,
	.priority	= INT_MAX,
};


int kbox_init_notifier(void)
{
	int ret = 0;
	
	ret = register_reboot_notifier(&kbox_reboot_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "register_reboot_notifier failed! ret = %d\n", ret); goto fail;);

	ret = register_die_notifier(&kbox_die_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "register_die_notifier failed! ret = %d\n", ret); goto fail;);

	ret = atomic_notifier_chain_register(&panic_notifier_list, &kbox_panic_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "atomic_notifier_chain_register failed! ret = %d\n", ret); goto fail;);

	return 0;

fail:
	kbox_cleanup_notifier();

	return ret;
}

void kbox_cleanup_notifier(void)
{
	int ret = 0;
	
	ret = unregister_reboot_notifier(&kbox_reboot_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "unregister_reboot_notifier failed! ret = %d\n", ret););

	ret = unregister_die_notifier(&kbox_die_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "unregister_die_notifier failed! ret = %d\n", ret););

	ret = atomic_notifier_chain_unregister(&panic_notifier_list, &kbox_panic_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "atomic_notifier_chain_unregister failed! ret = %d\n", ret););
       
}



