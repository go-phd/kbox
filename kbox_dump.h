
#ifndef _KBOX_DUMP_H_
#define _KBOX_DUMP_H_

#include "kbox.h"

enum kbox_error_type_e {
	KBOX_MCE_EVENT = 1,
	KBOX_OPPS_EVENT,
	KBOX_PANIC_EVENT,
	KBOX_DIE_EVENT,
	KBOX_REBOOT_EVENT
};

void kbox_dump_debug_print(void);
void kbox_dump_event(enum kbox_error_type_e type, unsigned long event,
		     const char *msg);

int kbox_init_dump(void);
void kbox_cleanup_dump(void);

#endif
