
#include <asm/io.h>
#include "kbox_ram_image.h"

void __iomem *kbox_base_addr = NULL;

void __iomem *kbox_get_section_addr(enum kbox_section_e  kbox_section)
{
	void __iomem *kbox_addr = kbox_base_addr;
	unsigned long kbox_len = KBOX_RESERVERED_MEMORY_LEN;

	if (!kbox_addr) {
		KBOX_LOG(KLOG_ERROR, "get kbox_addr or kbox_len failed!\n");
		return NULL;
	}

	switch (kbox_section) {
	case KBOX_SECTION_KERNEL:
		return kbox_addr;

	case KBOX_SECTION_PANIC:
		return kbox_addr + SECTION_KERNEL_LEN;

	case KBOX_SECTION_THREAD:
		return kbox_addr + SECTION_KERNEL_LEN + SECTION_PANIC_LEN;

	case KBOX_SECTION_PRINTK1:
		return kbox_addr + (kbox_len - (2 * SECTION_PRINTK_LEN) -
				    SECTION_USER_LEN);

	case KBOX_SECTION_PRINTK2:
		return kbox_addr + (kbox_len - SECTION_PRINTK_LEN -
				    SECTION_USER_LEN);

	case KBOX_SECTION_USER:
		return kbox_addr + (kbox_len - SECTION_USER_LEN);

	case KBOX_SECTION_ALL:
		return kbox_addr;

	default:
		KBOX_LOG(KLOG_ERROR, "input kbox_section error!\n");
		return NULL;
	}
}

unsigned long kbox_get_section_len(enum kbox_section_e  kbox_section)
{
	unsigned long kbox_len = KBOX_RESERVERED_MEMORY_LEN;

	switch (kbox_section) {
	case KBOX_SECTION_KERNEL:
		return SECTION_KERNEL_LEN;

	case KBOX_SECTION_PANIC:
		return SECTION_PANIC_LEN;

	case KBOX_SECTION_THREAD:
		return (kbox_len - (2 * SECTION_PRINTK_LEN) -
			SECTION_USER_LEN - SECTION_KERNEL_LEN -
			SECTION_PANIC_LEN);

	case KBOX_SECTION_PRINTK1:
	case KBOX_SECTION_PRINTK2:
		return SECTION_PRINTK_LEN;

	case KBOX_SECTION_USER:
		return SECTION_USER_LEN;

	case KBOX_SECTION_ALL:
		return kbox_len;

	default:
		KBOX_LOG(KLOG_ERROR, "input kbox_section error!\n");
		return 0;
	}

	return 0;
}

unsigned long kbox_get_section_phy_addr(enum kbox_section_e  kbox_section)
{
	unsigned long kbox_phy_addr = KBOX_RESERVERED_MEMORY;
	unsigned long kbox_len = KBOX_RESERVERED_MEMORY_LEN;

	switch (kbox_section) {
	case KBOX_SECTION_KERNEL:
		return kbox_phy_addr;

	case KBOX_SECTION_PANIC:
		return kbox_phy_addr + SECTION_KERNEL_LEN;

	case KBOX_SECTION_THREAD:
		return kbox_phy_addr + SECTION_KERNEL_LEN + SECTION_PANIC_LEN;

	case KBOX_SECTION_PRINTK1:
		return kbox_phy_addr + (kbox_len - (2 * SECTION_PRINTK_LEN) -
					SECTION_USER_LEN);

	case KBOX_SECTION_PRINTK2:
		return kbox_phy_addr + (kbox_len - SECTION_PRINTK_LEN -
					SECTION_USER_LEN);

	case KBOX_SECTION_USER:
		return kbox_phy_addr + (kbox_len - SECTION_USER_LEN);

	case KBOX_SECTION_ALL:
		return kbox_phy_addr;

	default:
		KBOX_LOG(KLOG_ERROR, "input kbox_section error!\n");
		return 0;
	}
}

void kbox_init_ram_image(void)
{
	if (kbox_base_addr == NULL) {
		kbox_base_addr = ioremap(KBOX_RESERVERED_MEMORY, KBOX_RESERVERED_MEMORY_LEN);
	}
}

void kbox_cleanup_ram_image(void)
{
	if (kbox_base_addr) {
		iounmap(kbox_base_addr);
		kbox_base_addr = NULL;
	}
}


