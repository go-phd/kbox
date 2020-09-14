#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/security.h>


#include "kbox_lsm.h"

static int kbox_capable(const struct cred *cred, struct user_namespace *ns,
			   int cap, int audit)
{
	if (cap == CAP_SYS_BOOT) {
		//record reboot log
	}

	return 0;
}

static struct security_operations kbox_lsm_ops = {
	.name =			"kbox",
	.capable =			kbox_capable,
};


int kbox_init_lsm(void)
{
	int ret = 0;
	
	ret = register_security(&kbox_lsm_ops);
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret, 
        KBOX_LOG(KLOG_ERROR, "register_security kbox_lsm_ops failed! ret = %d\n", ret););
	
	return ret;
}

int kbox_cleanup_lsm(void)
{
	int ret = 0;
	

	return ret;
}



