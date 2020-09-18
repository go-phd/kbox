
#include <linux/console.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#include "kbox_ram_op.h"
#include "kbox_console.h"

static char *g_printk_info_buf;
static char *g_printk_info_buf_tmp;

static struct kbox_ctrl_block_tmp_s g_printk_ctrl_block_tmp = { };

//static DEFINE_SPINLOCK(g_printk_buf_lock);

void kbox_console_debug_print(void)
{
	//int i = 0;

	KBOX_LOG(KLOG_ERROR, "g_printk_info_buf_tmp = %s\n", g_printk_info_buf_tmp);
	KBOX_LOG(KLOG_ERROR, "g_printk_info_buf = %s\n", g_printk_info_buf);
	KBOX_LOG(KLOG_ERROR, "idx = %d, len = %d\n", g_printk_ctrl_block_tmp.idx, g_printk_ctrl_block_tmp.valid_len);
	/*for (i = 0; i < g_printk_ctrl_block_tmp.valid_len; i++) {
		printk(KERN_ALERT "0x%x", g_printk_info_buf[i]);
	}*/
}

void kbox_output_printk_info(void)
{
	if (g_printk_ctrl_block_tmp.valid_len == 0) {
		return;
	}

	if (g_printk_ctrl_block_tmp.valid_len < SECTION_PRINTK_LEN) {
		memcpy(g_printk_info_buf_tmp, g_printk_info_buf, g_printk_ctrl_block_tmp.valid_len);
	} else {
		int last_len = SECTION_PRINTK_LEN - g_printk_ctrl_block_tmp.idx;
		memcpy(g_printk_info_buf_tmp, g_printk_info_buf + g_printk_ctrl_block_tmp.idx, last_len);
		memcpy(g_printk_info_buf_tmp + last_len, g_printk_info_buf, g_printk_ctrl_block_tmp.idx);
	}

	(void)kbox_write_printk_info(g_printk_info_buf_tmp, g_printk_ctrl_block_tmp.valid_len);
}

static void kbox_console_write(struct console *co, const char *buf, unsigned count)
{	
	if (unlikely(!g_printk_info_buf)) {
		return;
	}

	UNUSED(co);

	if (g_printk_ctrl_block_tmp.idx + count < SECTION_PRINTK_LEN) {
		memcpy(g_printk_info_buf + g_printk_ctrl_block_tmp.idx, buf, count);
	} else {
		int last_len = g_printk_ctrl_block_tmp.idx + count - SECTION_PRINTK_LEN;
		int frist_len = count - last_len;

		memcpy(g_printk_info_buf + g_printk_ctrl_block_tmp.idx, buf, frist_len);
		memcpy(g_printk_info_buf, buf + frist_len, last_len);
	}

	g_printk_ctrl_block_tmp.idx = (g_printk_ctrl_block_tmp.idx + count) & (SECTION_PRINTK_LEN - 1);

	g_printk_ctrl_block_tmp.valid_len += count;
	if (g_printk_ctrl_block_tmp.valid_len > SECTION_PRINTK_LEN) {
		g_printk_ctrl_block_tmp.valid_len = SECTION_PRINTK_LEN;
	}
}


static struct console kbox_console = {
	.name    = "KboxConsole",
	.write   = kbox_console_write,
	.flags   = CON_ENABLED | CON_PRINTBUFFER,
};


int kbox_init_console(void)
{
	int ret = 0;

	g_printk_info_buf = kzalloc(SECTION_PRINTK_LEN,
				    GFP_KERNEL);
	if (IS_ERR(g_printk_info_buf) || !g_printk_info_buf) {
		KBOX_LOG(KLOG_ERROR, "kmalloc g_printk_info_buf fail!\n");
		return -ENOMEM;
	}

	g_printk_info_buf_tmp = kzalloc(SECTION_PRINTK_LEN,
					GFP_KERNEL);
	if (IS_ERR(g_printk_info_buf_tmp) || !g_printk_info_buf_tmp) {
		KBOX_LOG(KLOG_ERROR, "kmalloc g_printk_info_buf_tmp fail!\n");
		ret = -ENOMEM;
		goto fail;
	}

	memset(&g_printk_ctrl_block_tmp, 0, sizeof(struct kbox_ctrl_block_tmp_s));

	register_console(&kbox_console);
	
	return 0;
fail:
	if (g_printk_info_buf) {
		kfree(g_printk_info_buf);
		g_printk_info_buf = NULL;
	}
	
	if (g_printk_info_buf_tmp) {
		kfree(g_printk_info_buf_tmp);
		g_printk_info_buf_tmp = NULL;
	}
	
	return ret;
}

void kbox_cleanup_console(void)
{
	int ret = 0;

	if (g_printk_info_buf) {
		kfree(g_printk_info_buf);
		g_printk_info_buf = NULL;
	}
	
	if (g_printk_info_buf_tmp) {
		kfree(g_printk_info_buf_tmp);
		g_printk_info_buf_tmp = NULL;
	}
	
	ret = unregister_console(&kbox_console);
	DO_INFO_IF_EXPR_UNLIKELY(ret,
			KBOX_LOG(KLOG_ERROR, "kbox_printk_init_info failed! ret = %d\n", ret););
}



