
#include <linux/spinlock.h>
#include <linux/utsname.h>		/* system_utsname */
#include <linux/rtc.h>		/* struct rtc_time */
#include <linux/slab.h>
#include <phd/kbox.h>

#include "kbox_ram_image.h"
#include "kbox_ram_op.h"
#include "kbox_console.h"
#include "kbox_dump.h"


#define PANIC_TMP_BUF_SIZE 256

static char *g_panic_info_buf_tmp;
static char *g_panic_info_buf;
static struct kbox_ctrl_block_tmp_s g_panic_ctrl_block_tmp = { };
static DEFINE_SPINLOCK(g_panic_buf_lock);

void kbox_dump_debug_print(void)
{
	//int i = 0;

	KBOX_LOG(KLOG_ERROR, "g_panic_info_buf_tmp = %s\n", g_panic_info_buf_tmp);
	KBOX_LOG(KLOG_ERROR, "g_panic_info_buf = %s\n", g_panic_info_buf);
	KBOX_LOG(KLOG_ERROR, "idx = %d, len = %d\n", g_panic_ctrl_block_tmp.idx, g_panic_ctrl_block_tmp.valid_len);
	/*for (i = 0; i < g_printk_ctrl_block_tmp.valid_len; i++) {
		printk(KERN_ALERT "0x%x", g_printk_info_buf[i]);
	}*/
}


static void kbox_output_syslog_info(void)
{
	unsigned long flags = 0;
	
	spin_lock_irqsave(&g_panic_buf_lock, flags);

	if (g_panic_ctrl_block_tmp.valid_len == 0) {
		spin_unlock_irqrestore(&g_panic_buf_lock, flags);
		return;
	}

	if (g_panic_ctrl_block_tmp.valid_len < SECTION_PANIC_LEN) {
		memcpy(g_panic_info_buf_tmp, g_panic_info_buf, g_panic_ctrl_block_tmp.valid_len);
	} else {
		int last_len = SECTION_PANIC_LEN - g_panic_ctrl_block_tmp.idx;
		memcpy(g_panic_info_buf_tmp, g_panic_info_buf + g_panic_ctrl_block_tmp.idx, last_len);
		memcpy(g_panic_info_buf_tmp + last_len, g_panic_info_buf, g_panic_ctrl_block_tmp.idx);
	}

	spin_unlock_irqrestore(&g_panic_buf_lock, flags);
	
	(void)kbox_write_panic_info(g_panic_info_buf_tmp, g_panic_ctrl_block_tmp.valid_len);
}

static void kbox_duplicate_syslog_info(const char *buf, unsigned int count)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&g_panic_buf_lock, flags);

	if (g_panic_ctrl_block_tmp.idx + count < SECTION_PANIC_LEN) {
		memcpy(g_panic_info_buf + g_panic_ctrl_block_tmp.idx, buf, count);
	} else {
		int last_len = g_panic_ctrl_block_tmp.idx + count - SECTION_PANIC_LEN;
		int frist_len = count - last_len;

		memcpy(g_panic_info_buf + g_panic_ctrl_block_tmp.idx, buf, frist_len);
		memcpy(g_panic_info_buf, buf + frist_len, last_len);
	}

	g_panic_ctrl_block_tmp.idx = (g_panic_ctrl_block_tmp.idx + count) & (SECTION_PANIC_LEN - 1);

	g_panic_ctrl_block_tmp.valid_len += count;
	if (g_panic_ctrl_block_tmp.valid_len > SECTION_PANIC_LEN) {
		g_panic_ctrl_block_tmp.valid_len = SECTION_PANIC_LEN;
	}

	spin_unlock_irqrestore(&g_panic_buf_lock, flags);
}
				  
void kbox_dump_painc_info(const char *fmt, ...)
{
	va_list args;
	int num = 0;
	char tmp_buf[PANIC_TMP_BUF_SIZE] = { };

	va_start(args, fmt);

	num = vsnprintf(tmp_buf, sizeof(tmp_buf) - 1, fmt, args);
	if (num >= 0) {
		kbox_duplicate_syslog_info(tmp_buf, num);
	}
	va_end(args);
}

static void kbox_show_kernel_version(void)
{
	kbox_dump_painc_info
		("\nOS : %s,\nRelease : %s,\nVersion : %s,\n",
		 init_uts_ns.name.sysname,
		 init_uts_ns.name.release,
		 init_uts_ns.name.version);
	kbox_dump_painc_info
		("Machine : %s,\nNodename : %s\n",
		 init_uts_ns.name.machine,
		 init_uts_ns.name.nodename);
}

static void kbox_show_version(void)
{
	kbox_dump_painc_info("\nKBOX_VERSION         : %s\n",
				   KBOX_VERSION);
}

static void kbox_show_time_stamps(void)
{
	struct rtc_time rtc_time_val = { };
	struct timespec64 uptime;

	ktime_get_coarse_real_ts64(&uptime);
	rtc_time64_to_tm(uptime.tv_sec, &rtc_time_val);

	kbox_dump_painc_info
		("\nCurrent time         : %04d-%02d-%02d %02d:%02d:%02d\n",
		 rtc_time_val.tm_year + 1900, rtc_time_val.tm_mon + 1,
		 rtc_time_val.tm_mday, rtc_time_val.tm_hour,
		 rtc_time_val.tm_min, rtc_time_val.tm_sec);
}

void kbox_dump_event(enum kbox_error_type_e type, unsigned long event,
		     const char *msg)
{
	kbox_dump_painc_info("\n====kbox begin dumping====\n");

	switch (type) {
	case KBOX_REBOOT_EVENT:
		kbox_dump_painc_info("reboot, event: 0x%lx, msg: %s\n", event, msg);
		break;
	case KBOX_DIE_EVENT:
		kbox_dump_painc_info("die, event: 0x%lx, msg: %s\n", event, msg);
		break;
	case KBOX_PANIC_EVENT:	
		kbox_dump_painc_info("panic event: 0x%lx, msg: %s\n", event, msg);
		break;
	default:
		break;
	}

	kbox_show_kernel_version();

	kbox_show_version();

	kbox_show_time_stamps();

	kbox_dump_painc_info("\n====kbox end dump====\n");

	kbox_output_syslog_info();
	kbox_output_printk_info();
}

 int kbox_init_dump(void)
 {
 	int ret = 0;
	
	 g_panic_info_buf = kzalloc(SECTION_PANIC_LEN, GFP_KERNEL);
	 if (IS_ERR(g_panic_info_buf) || !g_panic_info_buf) {
		 KBOX_LOG(KLOG_ERROR, "kmalloc g_panic_info_buf fail!\n");
		 ret = -ENOMEM;
		 goto fail;
	 }

	 g_panic_info_buf_tmp = kzalloc(SECTION_PANIC_LEN, GFP_KERNEL);
	 if (IS_ERR(g_panic_info_buf_tmp) || !g_panic_info_buf_tmp) {
		 KBOX_LOG(KLOG_ERROR, "kmalloc g_panic_info_buf_tmp fail!\n");
		 ret = -ENOMEM;
		 goto fail;
	 }

	 memset(&g_panic_ctrl_block_tmp, 0, sizeof(struct kbox_ctrl_block_tmp_s));
 
	 return ret;
 fail:

 	if (g_panic_info_buf) {
		kfree(g_panic_info_buf);
	 	g_panic_info_buf = NULL;
	}
	 
 	if (g_panic_info_buf_tmp) {
		kfree(g_panic_info_buf_tmp);
		g_panic_info_buf_tmp = NULL;		
	}

	 return ret;
 }

 void kbox_cleanup_dump(void)
 {
	if (g_panic_info_buf) {
		kfree(g_panic_info_buf);
	 	g_panic_info_buf = NULL;
	}
	 
 	if (g_panic_info_buf_tmp) {
		kfree(g_panic_info_buf_tmp);
		g_panic_info_buf_tmp = NULL;		
	}
 }
