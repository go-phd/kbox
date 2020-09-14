#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <asm/mce.h>


#include "kbox_mce.h"
#include "kbox_console.h"


static int kbox_mce_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	char str_buf[1024] = {};
	int count = 0;

	kbox_enable_console_write();
	
	count += sprintf(str_buf, "======mce event start======\n");
	count += sprintf(str_buf + count, "---do some thing---\n");
	count += sprintf(str_buf + count, "======mce event end======\n");
	
	
	kbox_write_to_syscom(str_buf, count);
	//KBOX_LOG(KLOG_ERROR, "event = 0x%x\n", event);
	return NOTIFY_DONE;
}


static struct notifier_block kbox_mce_block = {
	.notifier_call	= kbox_mce_event,
	.priority	= INT_MAX,
};

int kbox_init_mce_notifier(void)
{
	int ret = 0;
	
	mce_register_decode_chain(&kbox_mce_block);

	return ret;
}

int kbox_cleanup_mce_notifier(void)
{
	int ret = 0;
	
	mce_unregister_decode_chain(&kbox_mce_block);

	return ret;
}



