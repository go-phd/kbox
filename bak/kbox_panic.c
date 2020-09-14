#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

#include "kbox_panic.h"
#include "kbox_collect.h"
#include "kbox_console.h"


static int kbox_panic_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	int ret = 0;
	unsigned char *process_list = NULL;
	unsigned char *memory_list = NULL;
	char str_buf[1024] = {};
	int count = 0;

	kbox_enable_console_write();
	
	count += sprintf(str_buf, "======panic event start======\n");
	count += sprintf(str_buf + count, "---do some thing---\n");
	count += sprintf(str_buf + count, "======panic event end======\n");

	ret = kbox_collect_process_list(&process_list);
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_collect_process_list failed! ret = %d\n", ret););

	ret = kbox_collect_memory_list(&memory_list);
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_collect_memory_list failed! ret = %d\n", ret););
	
	
	kbox_write_to_syscom(str_buf, count);
	//KBOX_LOG(KLOG_ERROR, "event = 0x%x\n", event);
	//output to BMC
	return NOTIFY_DONE;
}


static struct notifier_block kbox_panic_block = {
	.notifier_call	= kbox_panic_event,
	.priority	= INT_MAX,
};

int kbox_init_panic_notifier(void)
{
	int ret = 0;
	
	ret = atomic_notifier_chain_register(&panic_notifier_list, &kbox_panic_block);
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret, 
        KBOX_LOG(KLOG_ERROR, "atomic_notifier_chain_register panic failed! ret = %d\n", ret););

	return ret;
}

int kbox_cleanup_panic_notifier(void)
{
	int ret = 0;
	
	ret = atomic_notifier_chain_unregister(&panic_notifier_list, &kbox_panic_block);
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret,
        KBOX_LOG(KLOG_ERROR, "atomic_notifier_chain_register panic failed! ret = %d\n", ret););

	return ret;
}



