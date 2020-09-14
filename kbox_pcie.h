
#ifndef _KBOX_PCIE_H
#define _KBOX_PCIE_H

#include "kbox.h"



int kbox_pcie_get_proeb_flag(void);
int kbox_pcie_send_request(unsigned char *data, unsigned short data_len);
int kbox_init_pcie(void);
void kbox_cleanup_pcie(void);


#endif


