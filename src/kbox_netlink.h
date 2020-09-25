
#ifndef _KBOX_NETLINK_H
#define _KBOX_NETLINK_H


#include <linux/module.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/err.h>


#define NETLINK_SSF     30

enum {
	KBOX_NLGRP_NONE,
	KBOX_NLGRP_DEVICE_EVENT,
	KBOX_NLGRP_SYSTEM_EVENT,
	KBOX_NLGRP_MAX
};

enum {
	KBOX_NL_CMD_NONE,
	KBOX_NL_CMD_REBOOT,
	KBOX_NL_CMD_DIE,
	KBOX_NL_CMD_PANIC,
	KBOX_NL_CMD_POWER_SUPPLY,
	KBOX_NL_CMD_MAX
};


struct kbox_nl_event_hdr_s {
	u32 event_type;
	u32 cmd;
	u32 reserve[2];
	u32 datalen;
	u8 data[0];
} __attribute__((aligned(sizeof(uint64_t))));

#define SIZE_OF_NL_EVNET_HDR (sizeof(struct kbox_nl_event_hdr_s))


#define DIR_USER_2_KERNEL 0
#define DIR_KERNEL_2_USER 1

struct kbox_nl_msg_hdr_s {
	u32 cmd;
	u32 dir;	// 0 u2k, 1 k2u
	u32 reserve[2];
	u32 datalen;
	u8 data[0];
} __attribute__((aligned(sizeof(uint64_t))));

#define SIZE_OF_NL_MSG_HDR (sizeof(struct kbox_nl_msg_hdr_s))


int kbox_init_netlink(void);
void kbox_cleanup_netlink(void);

int kbox_broadcast(u32 group, int cmd, void *msg, size_t size, gfp_t gfp_mask);





#endif

