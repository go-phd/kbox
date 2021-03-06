
#ifndef _KBOX_CDEV_H_
#define _KBOX_CDEV_H_

#include <linux/types.h>
#include <linux/atomic.h>
#include <phd/phdlsm.h>


#define KBOX_IOC_MAGIC (0x20)

#define GET_KBOX_TOTAL_LEN _IOR(KBOX_IOC_MAGIC, 0, unsigned long)

#define KBOX_TEST _IOW(KBOX_IOC_MAGIC, 1, unsigned long)

#define KBOX_ISM_SET_CTRL_PID _IOW(KBOX_IOC_MAGIC, 2, unsigned long)
#define KBOX_ISM_SET_CTRL_FILE _IOW(KBOX_IOC_MAGIC, 3, unsigned long)

#define KBOX_IOC_MAXNR 4


enum {
	KBOX_TEST_NONE,
	KBOX_TEST_NETLINK,
	KBOX_TEST_MAX
};

union kbox_ioctl_test_u {
	u32 nl_group;
};

struct kbox_ioctl_test_s {
	u32 type;
	union kbox_ioctl_test_u u;
};

struct kbox_ioctl_lsm_set_s {
	u32 type;
	char service_name[PHDLSM_NAME_MAX_LEN];
	char file_name[PHDLSM_FILE_PATH_MAX_LEN];
};

struct kbox_dev_s {
	atomic_t au_count;
};

int kbox_init_cdev(void);
void kbox_cleanup_cdev(void);

#endif
