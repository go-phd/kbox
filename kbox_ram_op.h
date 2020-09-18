
#ifndef _KBOX_RAM_OP_H_
#define _KBOX_RAM_OP_H_

#include <asm/ioctls.h>
#include <linux/fs.h>
#include "kbox_ram_image.h"


#define TEMP_BUF_SIZE (32 * 1024)
#define TEMP_BUF_DATA_SIZE (128 * 1024)
#define KBOX_RW_UNIT 4

struct kbox_region_arg_s {
	unsigned long offset;
	unsigned int count;
	char *data;
};

struct kbox_ctrl_block_tmp_s {
	unsigned int idx;			// current idx
	unsigned int valid_len;		// valid length 
};

int kbox_read_op(long long offset, unsigned int count, char __user *data);
int kbox_read_super_block(struct image_super_block_s *kbox_super_block);
int kbox_super_block_init(void);
int kbox_write_panic_info(const char *input_data, unsigned int data_len);
int kbox_write_printk_info(const char *input_data, unsigned int data_len);
int kbox_clear_region(enum kbox_section_e  section);
int kbox_write_to_ram(unsigned long offset, unsigned int count,
		      const char *data, enum kbox_section_e section);
int kbox_read_from_ram(unsigned long offset, unsigned int count, char *data,
		       enum kbox_section_e section);
int kbox_memset_ram(unsigned long offset, unsigned int count,
		    const char set_byte, enum kbox_section_e section);

#endif
