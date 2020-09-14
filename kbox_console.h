
#ifndef _KBOX_CONSOLE_H
#define _KBOX_CONSOLE_H

#include <linux/console.h>
#include "kbox.h"


void kbox_enable_console_write(void);
int kbox_init_console(void);
void kbox_cleanup_console(void);


#endif


