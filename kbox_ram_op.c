
#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/capability.h>
#include <linux/uaccess.h>		/* copy_*_user */
#include <linux/delay.h>		/* udelay */
#include <linux/mm.h>

#include "kbox_ram_image.h"
#include "kbox_ram_op.h"

static DEFINE_SPINLOCK(g_kbox_super_block_lock);
static DEFINE_SEMAPHORE(user_sem);


static struct image_super_block_s g_kbox_super_block = { };

void kbox_write_to_pci(void __iomem *dest, const void *src, int len,
		       unsigned long offset)
{
	memcpy(dest + offset, src, len);
}

void kbox_read_from_pci(void *dest, void __iomem *src, int len,
			unsigned long offset)
{
	memcpy(dest, src + offset, len);
}

void kbox_memset_pci(void __iomem *dest, const char set_byte, int len,
		     unsigned long offset)
{
	memset(dest + offset, set_byte, len);
}

int kbox_read_from_ram(unsigned long offset, unsigned int count, char *data,
		       enum kbox_section_e  section)
{
	unsigned int read_len_total = count;
	void __iomem *kbox_section_addr = kbox_get_section_addr(section);
	unsigned long kbox_section_len = kbox_get_section_len(section);

	if (!data) {
		KBOX_LOG(KLOG_ERROR, "input NULL point!\n");
		return -EFAULT;
	}

	if (!kbox_section_addr || kbox_section_len == 0) {
		KBOX_LOG(KLOG_ERROR, "get kbox_section_addr or kbox_section_len failed!\n");
		return -EFAULT;
	}

	if (offset >= kbox_section_len) {
		KBOX_LOG(KLOG_ERROR, "input offset is error!\n");
		return -EFAULT;
	}

	if ((offset + count) > kbox_section_len)
		read_len_total = (unsigned int)(kbox_section_len - offset);

	kbox_read_from_pci(data, kbox_section_addr, read_len_total, offset);

	return 0;
}

int kbox_write_to_ram(unsigned long offset, unsigned int count,
		      const char *data, enum kbox_section_e  section)
{
	unsigned int write_len_total = count;
	void __iomem *kbox_section_addr = kbox_get_section_addr(section);
	unsigned long kbox_section_len = kbox_get_section_len(section);

	if (!data) {
		KBOX_LOG(KLOG_ERROR, "input NULL point!\n");
		return -EFAULT;
	}

	if (!kbox_section_addr || kbox_section_len == 0) {
		KBOX_LOG(KLOG_ERROR, "get kbox_section_addr or kbox_section_len failed!\n");
		return -EFAULT;
	}

	if (offset >= kbox_section_len) {
		KBOX_LOG(KLOG_ERROR, "input offset is error!\n");
		return -EFAULT;
	}

	if ((offset + count) > kbox_section_len)
		write_len_total = (unsigned int)(kbox_section_len - offset);

	KBOX_LOG(KLOG_ERROR, "count = %d\n", count);

	kbox_write_to_pci(kbox_section_addr, data, write_len_total, offset);

	return 0;
}

int kbox_memset_ram(unsigned long offset, unsigned int count,
		    const char set_byte, enum kbox_section_e  section)
{
	unsigned int memset_len = count;
	void __iomem *kbox_section_addr = kbox_get_section_addr(section);
	unsigned long kbox_section_len = kbox_get_section_len(section);

	if (!kbox_section_addr || kbox_section_len == 0) {
		KBOX_LOG(KLOG_ERROR, "get kbox_section_addr or kbox_section_len failed!\n");
		return -EFAULT;
	}

	if (offset >= kbox_section_len) {
		KBOX_LOG(KLOG_ERROR, "input offset is error!\n");
		return -EFAULT;
	}

	if ((offset + count) > kbox_section_len)
		memset_len = (unsigned int)(kbox_section_len - offset);

	kbox_memset_pci(kbox_section_addr, set_byte, memset_len, offset);

	return 0;
}

int kbox_read_op(long long offset, unsigned int count, char __user *data)
{
	unsigned int read_bytes = 0;
	unsigned int read_len = 0;
	unsigned int left_len = count;
	char *user_buf = data;
	char *temp_buf_char = NULL;
	unsigned long offset_tmp = offset;

	if (!data) {
		KBOX_LOG(KLOG_ERROR, "input NULL point!\n");
		return -EFAULT;
	}

	if (down_interruptible(&user_sem) != 0)
		return -EFAULT;

	temp_buf_char = kmalloc(TEMP_BUF_DATA_SIZE, GFP_KERNEL);
	if (!temp_buf_char) {
		KBOX_LOG(KLOG_ERROR, "kmalloc temp_buf_char fail!\n");
		up(&user_sem);
		return -ENOMEM;
	}

	memset((void *)temp_buf_char, 0, TEMP_BUF_DATA_SIZE);

	while (1) {
		if (read_len >= count)
			break;

		read_bytes =
			(left_len >
			 TEMP_BUF_DATA_SIZE) ? TEMP_BUF_DATA_SIZE : left_len;

		if (kbox_read_from_ram
			(offset_tmp, read_bytes, temp_buf_char, KBOX_SECTION_ALL) < 0) {
			KBOX_LOG(KLOG_ERROR, "kbox_read_from_ram fail!\n");
			break;
		}

		if (copy_to_user(user_buf, temp_buf_char, read_bytes)) {
			KBOX_LOG(KLOG_ERROR, "copy_to_user fail!\n");
			break;
		}

		left_len -= read_bytes;
		read_len += read_bytes;
		user_buf += read_bytes;

		offset_tmp += read_bytes;
		memset((void *)temp_buf_char, 0, TEMP_BUF_DATA_SIZE);

		msleep(20);
	}

	kfree(temp_buf_char);

	up(&user_sem);

	return (int)read_len;
}


static int kbox_update_super_block(struct image_super_block_s *kbox_super_block)
{
	int ret = 0;

	ret = kbox_write_to_ram(0, (unsigned int)sizeof(struct image_super_block_s),
			      (char *)kbox_super_block, KBOX_SECTION_SUPER_BLOCK);
	if (ret < 0) {
		KBOX_LOG(KLOG_ERROR, "fail to write superblock data!\n");
		return ret;
	}

	return ret;
}

int kbox_read_super_block(struct image_super_block_s *kbox_super_block)
{
	int ret = 0;

	ret = kbox_read_from_ram(0, (unsigned int)sizeof(struct image_super_block_s),
			       (char *)kbox_super_block,
			       KBOX_SECTION_SUPER_BLOCK);
	if (ret < 0) {
		KBOX_LOG(KLOG_ERROR, "fail to get superblock data!\n");
		return ret;
	}

	return 0;
}

static unsigned char kbox_get_byte_order(void)
{
	unsigned short data_short = 0xB22B;
	unsigned char *data_char = (unsigned char *)&data_short;

	return (unsigned char)((*data_char == 0xB2) ? KBOX_BIG_ENDIAN :
			       KBOX_LITTLE_ENDIAN);
}


int kbox_super_block_init(void)
{
	int ret = 0;

	ret = kbox_read_super_block(&g_kbox_super_block);
	if (ret != 0) {
		KBOX_LOG(KLOG_ERROR, "kbox_read_super_block fail!\n");
		return ret;
	}

	if (!VALID_IMAGE(&g_kbox_super_block)) {
		memset((void *)&g_kbox_super_block, 0x00, sizeof(struct image_super_block_s));
		
		g_kbox_super_block.byte_order = kbox_get_byte_order();
		g_kbox_super_block.curr_idx = 0;
		g_kbox_super_block.version = IMAGE_VER;
		g_kbox_super_block.magic_flag = IMAGE_MAGIC;
		
	} else {
		// 交换保存区
		u8 curr_idx = g_kbox_super_block.curr_idx ? 0 : 1;
		g_kbox_super_block.curr_idx = curr_idx;
	}

	if (kbox_update_super_block(&g_kbox_super_block) != 0) {
		KBOX_LOG(KLOG_ERROR, "kbox_update_super_block failed!\n");
		return -1;
	}

	KBOX_LOG(KLOG_DEBUG, "g_kbox_super_block.curr_idx = %d\n", g_kbox_super_block.curr_idx);

	return 0;
}

int kbox_write_panic_info(const char *input_data, unsigned int data_len)
{
	int ret = 0;
	enum kbox_section_e section = KBOX_SECTION_PANIC1;
	unsigned int offset = 0;
	unsigned long flags = 0;
	unsigned int data_len_real = data_len;

	if (!input_data || data_len == 0) {
		KBOX_LOG(KLOG_ERROR, "input parameter error!\n");
		return -1;
	}

	// 不能超出panic区的长度
	if (data_len > SECTION_PANIC_LEN) {
		data_len_real = SECTION_PANIC_LEN;
	}

	spin_lock_irqsave(&g_kbox_super_block_lock, flags);

	if (g_kbox_super_block.curr_idx) {
		section = KBOX_SECTION_PANIC2;
	} else {
		section = KBOX_SECTION_PANIC1;
	}

	spin_unlock_irqrestore(&g_kbox_super_block_lock, flags);

	ret = kbox_write_to_ram(offset, data_len_real, input_data, section);
	if (ret < 0) {
		KBOX_LOG(KLOG_ERROR, "fail to save panic information!\n");
		return ret;
	}

	return 0;
}

int kbox_write_printk_info(const char *input_data, unsigned int data_len)
{
	int ret = 0;
	enum kbox_section_e section = KBOX_SECTION_PRINTK1;

	if (!input_data) {
		KBOX_LOG(KLOG_ERROR, "input parameter error!\n");
		return -1;
	}

	if (g_kbox_super_block.curr_idx) {
		section = KBOX_SECTION_PRINTK2;
	} else {
		section = KBOX_SECTION_PRINTK1;
	}

	ret = kbox_write_to_ram(0, data_len, input_data, section);
	if (ret < 0) {
		KBOX_LOG(KLOG_ERROR, "fail to save printk information!(1)\n");
		return -1;
	}

	return 0;
}


int kbox_clear_region(enum kbox_section_e  section)
{
	int ret = 0;
	unsigned long kbox_section_len = kbox_get_section_len(section);

	if (kbox_section_len == 0) {
		KBOX_LOG(KLOG_ERROR, "get kbox_section_len failed!\n");
		return -EFAULT;
	}

	ret = kbox_memset_ram(0, (unsigned int)kbox_section_len, 0, section);
	if (ret != 0) {
		KBOX_LOG(KLOG_ERROR, "kbox_memset_ram failed!\n");
		return -EFAULT;
	}

	return 0;
}



