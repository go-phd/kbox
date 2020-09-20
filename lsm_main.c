#include <linux/module.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/security.h>
#include <linux/lsm_hooks.h>


#include "lsm.h"

static int inode_unlink(struct inode *dir, struct dentry *dentry)
{
	return 0;
}

static struct security_hook_list lsm_hooks[] = {
   LSM_HOOK_INIT(inode_unlink, inode_unlink),
};

static int __init lsm_init(void)
{
    int ret = 0;
	security_add_hooks(lsm_hooks, ARRAY_SIZE(lsm_hooks), "lsm");

    return ret;
}


static void __exit lsm_exit(void)
{
}

module_init(lsm_init);
module_exit(lsm_exit);



MODULE_AUTHOR("YCWL TECHNOLOGIES CO., LTD.");
MODULE_DESCRIPTION("ISM DRIVER");
MODULE_LICENSE("GPL");


