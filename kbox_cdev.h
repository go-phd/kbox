
#ifndef _KBOX_CDEV_H_
#define _KBOX_CDEV_H_

#include <linux/types.h>
#include <linux/atomic.h>

#include "kbox.h"


#define KBOX_IOC_MAGIC (0x20)

#define GET_KBOX_TOTAL_LEN _IOR(KBOX_IOC_MAGIC, 0, unsigned long)
#define CLEAR_KBOX_REGION_ALL _IO(KBOX_IOC_MAGIC, 1)
#define KBOX_MODIFY_TEST _IOW(KBOX_IOC_MAGIC, 2, unsigned long)

#define KBOX_IOC_MAXNR 3


struct kbox_dev_s {
	atomic_t au_count;
};

int kbox_init_cdev(void);
void kbox_cleanup_cdev(void);

#endif
