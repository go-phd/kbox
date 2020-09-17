#include <linux/module.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

#include "kbox.h"
#include "kbox_console.h"
#include "kbox_notifier.h"
#include "kbox_netlink.h"
#include "kbox_output.h"
#include "kbox_cdev.h"
#include "kbox_monitor.h"


#define KBOX_VERSION "V1.0.0"
int debug_level = 0;


static int __init kbox_init(void)
{
    int ret = 0;

	ret = kbox_init_console();
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret,
        KBOX_LOG(KLOG_ERROR, "kbox_init_panic_notifier failed! ret = %d\n", ret););

	ret = kbox_init_output();
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "kbox_init_output failed! ret = %d\n", ret); goto fail;);

	ret = kbox_init_notifier();
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "kbox_init_notifier failed! ret = %d\n", ret); goto fail;);

	ret = kbox_init_netlink();
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "kbox_init_netlink failed! ret = %d\n", ret); goto fail;);

	ret = kbox_init_monitor();
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_init_netlink failed! ret = %d\n", ret); goto fail;);

	ret = kbox_init_cdev();
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_init_cdev failed! ret = %d\n", ret); goto fail;);

	KBOX_LOG(KLOG_DEBUG, "kbox init ok, version is %s\n", KBOX_VERSION);

	return 0;

fail:
	kbox_cleanup_console();
	kbox_cleanup_output();
	kbox_cleanup_notifier();
	kbox_cleanup_netlink();
	kbox_cleanup_monitor();

    return ret;
}


static void __exit kbox_exit(void)
{
	kbox_cleanup_cdev();
	kbox_cleanup_monitor();
	kbox_cleanup_netlink();
	kbox_cleanup_notifier();
	kbox_cleanup_output();
	kbox_cleanup_console();

	KBOX_LOG(KLOG_DEBUG, "kbox exit ok\n");
}

module_init(kbox_init);
module_exit(kbox_exit);

MODULE_AUTHOR("YCWL TECHNOLOGIES CO., LTD.");
MODULE_DESCRIPTION("KBOX DRIVER");
MODULE_LICENSE("GPL");
MODULE_VERSION(KBOX_VERSION);


