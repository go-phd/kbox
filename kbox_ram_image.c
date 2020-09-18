
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include "kbox_ram_image.h"

void __iomem *kbox_base_addr = NULL;

struct kbox_ram_image_s g_ram_image;

void __iomem *kbox_get_section_addr(enum kbox_section_e  kbox_section)
{
	void __iomem *kbox_addr = g_ram_image.vir_addr;

	if (!kbox_addr) {
		KBOX_LOG(KLOG_ERROR, "kbox_addr failed!\n");
		return NULL;
	}

	switch (kbox_section) {
	case KBOX_SECTION_KERNEL:
		return kbox_addr;

	case KBOX_SECTION_PANIC1:
		return kbox_addr + SECTION_PANIC1_OFFSET;

	case KBOX_SECTION_PRINTK1:
		return kbox_addr + SECTION_PRINTK1_OFFSET;

	case KBOX_SECTION_PANIC2:
		return kbox_addr + SECTION_PANIC2_OFFSET;

	case KBOX_SECTION_PRINTK2:
		return kbox_addr + SECTION_PRINTK2_OFFSET;

	case KBOX_SECTION_USER:
		return kbox_addr + SECTION_USER_OFFSET;

	case KBOX_SECTION_ALL:
		return kbox_addr;

	default:
		KBOX_LOG(KLOG_ERROR, "input kbox_section error!\n");
		return NULL;
	}
}

unsigned long kbox_get_section_len(enum kbox_section_e  kbox_section)
{
	switch (kbox_section) {
	case KBOX_SECTION_KERNEL:
		return SECTION_KERNEL_LEN;

	case KBOX_SECTION_PANIC1:
	case KBOX_SECTION_PANIC2:
		return SECTION_PANIC_LEN;

	case KBOX_SECTION_PRINTK1:
	case KBOX_SECTION_PRINTK2:
		return SECTION_PRINTK_LEN;

	case KBOX_SECTION_USER:
		return SECTION_USER_LEN;

	case KBOX_SECTION_ALL:
		return KBOX_RESERVERED_MEMORY_LEN;

	default:
		KBOX_LOG(KLOG_ERROR, "kbox_section error, %d\n", kbox_section);
		return 0;
	}

	return 0;
}

unsigned long kbox_get_section_phy_addr(enum kbox_section_e  kbox_section)
{
	unsigned long kbox_phy_addr = g_ram_image.phy_addr;
	
	if (!kbox_phy_addr) {
		KBOX_LOG(KLOG_ERROR, "kbox_phy_addr failed!\n");
		return 0;
	}

	switch (kbox_section) {
	case KBOX_SECTION_KERNEL:
		return kbox_phy_addr;

	case KBOX_SECTION_PANIC1:
		return kbox_phy_addr + SECTION_PANIC1_OFFSET;

	case KBOX_SECTION_PRINTK1:
		return kbox_phy_addr + SECTION_PRINTK1_OFFSET;

	case KBOX_SECTION_PANIC2:
		return kbox_phy_addr + SECTION_PANIC2_OFFSET;

	case KBOX_SECTION_PRINTK2:
		return kbox_phy_addr + SECTION_PRINTK2_OFFSET;

	case KBOX_SECTION_USER:
		return kbox_phy_addr + SECTION_USER_OFFSET;

	case KBOX_SECTION_ALL:
		return kbox_phy_addr;

	default:
		KBOX_LOG(KLOG_ERROR, "kbox_section error, %d\n", kbox_section);
		return 0;
	}
}

void kbox_init_ram_image(void)
{
	memset(&g_ram_image, 0, sizeof(struct kbox_ram_image_s));
	
	if (g_ram_image.vir_addr == NULL) {
		g_ram_image.len = KBOX_RESERVERED_MEMORY_LEN;
#ifdef RAM_IMAGE_TEST
		//g_ram_image.vir_addr = dma_zalloc_coherent(NULL, KBOX_RESERVERED_MEMORY_LEN,
		//				       &g_ram_image.phy_addr, GFP_KERNEL);
		g_ram_image.vir_addr = kzalloc(KBOX_RESERVERED_MEMORY_LEN, GFP_KERNEL);
		if (!g_ram_image.vir_addr) {
			KBOX_LOG(KLOG_ERROR, "Failed to allocate dma buff\n");
		}
#else
		g_ram_image.phy_addr = KBOX_RESERVERED_MEMORY;
		g_ram_image.vir_addr = ioremap(KBOX_RESERVERED_MEMORY, KBOX_RESERVERED_MEMORY_LEN);
#endif
	}

	KBOX_LOG(KLOG_DEBUG, "vir_addr = %p, phy_addr = 0x%llx\n", g_ram_image.vir_addr, g_ram_image.phy_addr);

}

void kbox_cleanup_ram_image(void)
{
	if (g_ram_image.vir_addr) {
#ifdef RAM_IMAGE_TEST
		//dma_free_coherent(NULL, KBOX_RESERVERED_MEMORY_LEN, g_ram_image.vir_addr,
		//		  g_ram_image.phy_addr);
		kfree(g_ram_image.vir_addr);
#else
		iounmap(g_ram_image.vir_addr);
#endif

		g_ram_image.vir_addr = NULL;
		g_ram_image.phy_addr = 0;

		g_ram_image.len = 0;
	}
}


