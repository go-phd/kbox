
#ifndef _KBOX_RAM_IMAGE_H_
#define _KBOX_RAM_IMAGE_H_

//#define RAM_IMAGE_TEST

// 保留内存物理起始地址
#define KBOX_RESERVERED_MEMORY	0xFF900000

// 保留内存容量, 剩余最后 4KB 防止越界
#define KBOX_RESERVERED_MEMORY_LEN	(0x6F0000 - 0x1000)

enum kbox_section_e {
	KBOX_SECTION_SUPER_BLOCK = 1,
	KBOX_SECTION_PANIC1 = 2,
	KBOX_SECTION_PRINTK1 = 4,
	KBOX_SECTION_PANIC2 = 3,
	KBOX_SECTION_PRINTK2 = 5,
	KBOX_SECTION_RESERVE = 6,
	KBOX_SECTION_USER = 7,
	KBOX_SECTION_ALL = 8
};

#define KBOX_BIG_ENDIAN (0x2B)
#define KBOX_LITTLE_ENDIAN (0xB2)
#define IMAGE_VER (0x0001)
#define IMAGE_MAGIC (0xB202C086)
#define VALID_IMAGE(x) (IMAGE_MAGIC == (x)->magic_flag)
#define SLOT_NUM (2)

struct panic_ctrl_block_s {
	unsigned char len;
};

struct printk_info_ctrl_block_s {
	unsigned int len;
};

struct image_super_block_s {
	u8 byte_order;
	u8 curr_idx;		// 当前有效的 panic和prontk 区域，0或1
	u16 version;
	u32 magic_flag;
	struct panic_ctrl_block_s panic_ctrl_blk[SLOT_NUM];
	struct printk_info_ctrl_block_s printk_ctrl_blk[SLOT_NUM];
}__attribute__((aligned));

// dump 区尺寸为 1MB
#define SECTION_SUPER_BLOCK_LEN 0x1000		// 4 KB
#define SECTION_PANIC_LEN 0x20000			// 128 KB
#define SECTION_PRINTK_LEN 0x40000			// 256 KB
#define SECTION_RESERVE_LEN (0x100000 - SECTION_SUPER_BLOCK_LEN - 2 * SECTION_PANIC_LEN - 2 * SECTION_PRINTK_LEN)

// 1MB 以外的都是用户区
#define SECTION_USER_LEN (KBOX_RESERVERED_MEMORY_LEN - 0x100000)

#define SECTION_SUPER_BLOCK_OFFSET (0)
#define SECTION_PANIC1_OFFSET SECTION_SUPER_BLOCK_LEN
#define SECTION_PRINTK1_OFFSET (SECTION_PANIC1_OFFSET + SECTION_PANIC_LEN)
#define SECTION_PANIC2_OFFSET (SECTION_PRINTK1_OFFSET + SECTION_PRINTK_LEN)
#define SECTION_PRINTK2_OFFSET (SECTION_PANIC2_OFFSET + SECTION_PANIC_LEN)
#define SECTION_RESERVE_OFFSET (SECTION_PRINTK2_OFFSET + SECTION_PRINTK_LEN)
#define SECTION_USER_OFFSET (SECTION_RESERVE_OFFSET + SECTION_RESERVE_LEN)

struct kbox_ram_image_s {
	dma_addr_t phy_addr;
	unsigned long len;
	void __iomem *vir_addr;
};


void __iomem *kbox_get_section_addr(enum kbox_section_e  kbox_section);
unsigned long kbox_get_section_len(enum kbox_section_e  kbox_section);
unsigned long kbox_get_section_phy_addr(enum kbox_section_e  kbox_section);
void kbox_init_ram_image(void);
void kbox_cleanup_ram_image(void);

#endif
