
#ifndef _KBOX_OUTPUT_H
#define _KBOX_OUTPUT_H

#include <linux/ipmi.h>
#include <linux/module.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/err.h>

#include "kbox.h"

void kbox_write_to_syscom(char *str, unsigned count);
int kbox_output(unsigned char *data, unsigned short data_len);
int kbox_init_output(void);
void kbox_cleanup_output(void);


#endif

