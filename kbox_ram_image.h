
#ifndef _KBOX_RAM_IMAGE_H_
#define _KBOX_RAM_IMAGE_H_


#include "kbox.h"


// 保留内存物理地址
#define KBOX_RESERVERED_MEMORY	0x12345678

// 保留内存容量
#define KBOX_RESERVERED_MEMORY_LEN	64 * 1024 * 1024

enum kbox_section_e {
	KBOX_SECTION_KERNEL = 1,
	KBOX_SECTION_PANIC = 2,
	KBOX_SECTION_THREAD = 3,
	KBOX_SECTION_PRINTK1 = 4,
	KBOX_SECTION_PRINTK2 = 5,
	KBOX_SECTION_USER = 6,
	KBOX_SECTION_ALL = 7
};

#define KBOX_BIG_ENDIAN (0x2B)
#define KBOX_LITTLE_ENDIAN (0xB2)
#define IMAGE_VER (0x0001)
#define IMAGE_MAGIC (0xB202C086)
#define VALID_IMAGE(x) (IMAGE_MAGIC == (x)->magic_flag)
#define SLOT_NUM (8)
#define SLOT_LENGTH (16 * 1024)
#define MAX_RECORD_NO (0xFF)
#define MAX_USE_NUMS (0xFF)

#define PRINTK_NUM (2)
#define PRINTK_CURR_FLAG ("curr")
#define PRINTK_LAST_FLAG ("last")
#define PRINTK_FLAG_LEN (4)

struct panic_ctrl_block_s {
	unsigned char use_nums;
	unsigned char number;
	unsigned short len;
	unsigned int time;
};

struct thread_info_ctrl_block_s {
	unsigned int thread_info_len;
};

struct printk_info_ctrl_block_s {
	unsigned char flag[PRINTK_FLAG_LEN];
	unsigned int len;
};

struct image_super_block_s {
	unsigned char byte_order;
	unsigned char checksum;
	unsigned short version;
	unsigned int magic_flag;
	unsigned int panic_nums;
	struct panic_ctrl_block_s panic_ctrl_blk[SLOT_NUM];
	struct printk_info_ctrl_block_s printk_ctrl_blk[PRINTK_NUM];
	struct thread_info_ctrl_block_s thread_ctrl_blk;
};

#define SECTION_KERNEL_LEN (sizeof(struct image_super_block_s))
#define SECTION_PANIC_LEN (8 * SLOT_LENGTH)
#define SECTION_PRINTK_LEN (512 * 1024)
#define SECTION_USER_LEN (2 * 1024 * 1024)

#define SECTION_KERNEL_OFFSET (0)
#define SECTION_PANIC_OFFSET SECTION_KERNEL_LEN
#define SECTION_THREAD_OFFSET (SECTION_KERNEL_LEN + SECTION_PANIC_LEN)

void __iomem *kbox_get_section_addr(enum kbox_section_e  kbox_section);
unsigned long kbox_get_section_len(enum kbox_section_e  kbox_section);
unsigned long kbox_get_section_phy_addr(enum kbox_section_e  kbox_section);
void kbox_init_ram_image(void);
void kbox_cleanup_ram_image(void);

#endif
