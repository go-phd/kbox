
#ifndef _KBOX_NOTIFIER_H
#define _KBOX_NOTIFIER_H

#include <linux/notifier.h>

int kbox_init_notifier(void);
void kbox_cleanup_notifier(void);

int kbox_panic_event(struct notifier_block *this,
		unsigned long event, void *ptr);


#endif


