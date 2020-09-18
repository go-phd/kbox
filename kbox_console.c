
#include <linux/console.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#include "kbox_ram_op.h"
#include "kbox_console.h"

static char *g_printk_info_buf;
static char *g_printk_info_buf_tmp;

static struct printk_ctrl_block_tmp_s g_printk_ctrl_block_tmp = { };

static DEFINE_SPINLOCK(g_printk_buf_lock);

void kbox_console_debug_print(void)
{
	//int i = 0;

	KBOX_LOG(KLOG_ERROR, "g_printk_info_buf = %s\n", g_printk_info_buf);
	KBOX_LOG(KLOG_ERROR, "start = %d, end = %d, len = %d\n", g_printk_ctrl_block_tmp.start, g_printk_ctrl_block_tmp.end, g_printk_ctrl_block_tmp.valid_len);
	/*for (i = 0; i < g_printk_ctrl_block_tmp.valid_len; i++) {
		printk(KERN_ALERT "0x%x", g_printk_info_buf[i]);
	}*/
}

void kbox_output_printk_info(void)
{
	unsigned int start_tmp = 0;
	unsigned int end_tmp = 0;
	unsigned int len_tmp = 0;
	unsigned long flags = 0;

	if (unlikely(!g_printk_info_buf || !g_printk_info_buf_tmp))
		return;

	spin_lock_irqsave(&g_printk_buf_lock, flags);
	if (g_printk_ctrl_block_tmp.valid_len == 0) {
		spin_unlock_irqrestore(&g_printk_buf_lock, flags);
		return;
	}

	start_tmp = (g_printk_ctrl_block_tmp.start % SECTION_PRINTK_LEN);
	end_tmp = ((g_printk_ctrl_block_tmp.end - 1) % SECTION_PRINTK_LEN);
	len_tmp = g_printk_ctrl_block_tmp.valid_len;

	if (start_tmp > end_tmp) {
		memcpy(g_printk_info_buf_tmp,
		       g_printk_info_buf + start_tmp,
			len_tmp - start_tmp);
		memcpy(g_printk_info_buf_tmp + len_tmp - start_tmp,
		       g_printk_info_buf,
			end_tmp + 1);
	} else {
		memcpy(g_printk_info_buf_tmp,
		       g_printk_info_buf + start_tmp,
			len_tmp);
	}

	spin_unlock_irqrestore(&g_printk_buf_lock, flags);

	(void)kbox_write_printk_info(g_printk_info_buf_tmp, &g_printk_ctrl_block_tmp);
}


static void kbox_emit_printk_char(const char c)
{
	if (unlikely(!g_printk_info_buf))
		return;

	*(g_printk_info_buf + (g_printk_ctrl_block_tmp.end % SECTION_PRINTK_LEN)) = c;
	g_printk_ctrl_block_tmp.end++;

	if (g_printk_ctrl_block_tmp.end > SECTION_PRINTK_LEN) {
		g_printk_ctrl_block_tmp.start++;
	}

	if (g_printk_ctrl_block_tmp.end < SECTION_PRINTK_LEN) {
		g_printk_ctrl_block_tmp.valid_len++;
	}
}

static void kbox_console_write(struct console *co, const char *buf, unsigned count)
{
	unsigned int idx = 0;
	unsigned long flags = 0;

	UNUSED(co);

	spin_lock_irqsave(&g_printk_buf_lock, flags);
	for (idx = 0; idx < count; idx++) {
		kbox_emit_printk_char(*buf++);
	}
	spin_unlock_irqrestore(&g_printk_buf_lock, flags);
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

	memset(&g_printk_ctrl_block_tmp, 0, sizeof(struct printk_ctrl_block_tmp_s));

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



