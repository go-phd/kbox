
#ifndef _KBOX_NOTIFIER_H
#define _KBOX_NOTIFIER_H

#include <linux/notifier.h>

int kbox_init_notifier(void);
void kbox_cleanup_notifier(void);

struct kbox_notifier_s {
	int sb2_dbg_irq;
};

#endif


