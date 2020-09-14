
#ifndef _KBOX_IPMI_H
#define _KBOX_IPMI_H

#include "kbox.h"

int kbox_ipmi_send_request(unsigned char *data, unsigned short data_len);
int kbox_init_ipmi(void);
void kbox_cleanup_ipmi(void);


#endif


