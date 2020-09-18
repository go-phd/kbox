
#ifndef _KBOX_CONSOLE_H
#define _KBOX_CONSOLE_H

#include "kbox_ram_image.h"
#include "kbox.h"

void kbox_console_debug_print(void);
void kbox_output_printk_info(void);


int kbox_init_console(void);
void kbox_cleanup_console(void);


#endif


