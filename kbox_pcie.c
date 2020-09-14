
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/pci.h>

#include "kbox_pcie.h"

#define PCI_VENDOR_KBOX                      0x1044//0x1556
#define PCI_DEVICE_KBOX                      0x9065//0xbb00


static int g_pcie_proeb_ok = 0;


int kbox_pcie_get_proeb_flag(void)
{
	return g_pcie_proeb_ok;
}

int kbox_pcie_send_request(unsigned char *data, unsigned short data_len)
{

  return 0;
}

static int kbox_probe(struct pci_dev *pci_dev, const struct pci_device_id *id)
{
	g_pcie_proeb_ok = 1;
    return 0;
}


void kbox_remove(struct pci_dev *pci_dev)
{

}


static struct pci_device_id kbox_pci_ids [] = {
    { PCI_DEVICE(PCI_VENDOR_KBOX, PCI_DEVICE_KBOX), },
    { 0, }
};

static struct pci_driver kbox_pci_driver = {
    .name = "kbox_pci",
    .id_table = kbox_pci_ids,
    .probe = kbox_probe,
    .remove = kbox_remove,
};


int kbox_init_pcie(void)
{
	return pci_register_driver(&kbox_pci_driver);
}

void kbox_cleanup_pcie(void)
{
	pci_unregister_driver(&kbox_pci_driver);
}



