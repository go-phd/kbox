#include <linux/kernel.h>
#include <linux/kdebug.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

#include "kbox_die.h"
#include "kbox_console.h"


static int kbox_die_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	char str_buf[1024] = {};
	int count = 0;

	kbox_enable_console_write();
	
	count += sprintf(str_buf, "=die event=\n");
	//count += sprintf(str_buf + count, "---do some thing---\n");
	//count += sprintf(str_buf + count, "======die event end======\n");
	
	
	kbox_write_to_syscom(str_buf, count);
	//KBOX_LOG(KLOG_ERROR, "event = 0x%x\n", event);
	return NOTIFY_DONE;
}


static struct notifier_block kbox_die_block = {
	.notifier_call	= kbox_die_event,
	.priority	= INT_MAX,
};

int kbox_init_die_notifier(void)
{
	int ret = 0;
	
	ret = register_die_notifier(&kbox_die_block);
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret, 
        KBOX_LOG(KLOG_ERROR, "register_die_notifier failed! ret = %d\n", ret););
	
	return ret;
}

int kbox_cleanup_die_notifier(void)
{
	int ret = 0;
	
	ret = unregister_die_notifier(&kbox_die_block);
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret, 
        KBOX_LOG(KLOG_ERROR, "unregister_die_notifier failed! ret = %d\n", ret););
	
	return ret;
}



