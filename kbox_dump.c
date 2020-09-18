
#include <linux/spinlock.h>
#include <linux/utsname.h>		/* system_utsname */
#include <linux/rtc.h>		/* struct rtc_time */
#include <linux/slab.h>

#include "kbox_ram_image.h"
#include "kbox_ram_op.h"
#include "kbox_console.h"
#include "kbox_dump.h"


#define PANIC_TMP_BUF_SIZE 256

static char *g_panic_info_buf_tmp;
static char *g_panic_info_buf;

static unsigned int g_panic_info_start;

static unsigned int g_panic_info_end;

static unsigned int g_panic_info_len;


#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)
#define LEAP_YEAR(year) \
	((!((year) % 4) && ((year) % 100)) || !((year) % 400))
#define MONTH_DAYS(month, year) \
	(g_day_in_month[(month)] + (int)(LEAP_YEAR(year) && (month == 1)))


static DEFINE_SPINLOCK(g_dump_lock);

static const char g_day_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static DEFINE_SPINLOCK(g_panic_buf_lock);


static void kbox_emit_syslog_char(const char c)
{
	if (unlikely(!g_panic_info_buf))
		return;

	*(g_panic_info_buf + (g_panic_info_end % SECTION_PANIC_LEN)) = c;
	g_panic_info_end++;

	if (g_panic_info_end > SECTION_PANIC_LEN)
		g_panic_info_start++;

	if (g_panic_info_len < SECTION_PANIC_LEN)
		g_panic_info_len++;
}

static int kbox_duplicate_syslog_info(const char *syslog_buf,
				      unsigned int buf_len)
{
	unsigned int idx = 0;
	unsigned long flags = 0;

	if (!syslog_buf)
		return 0;

	spin_lock_irqsave(&g_panic_buf_lock, flags);

	for (idx = 0; idx < buf_len; idx++)
		kbox_emit_syslog_char(*syslog_buf++);

	spin_unlock_irqrestore(&g_panic_buf_lock, flags);

	return buf_len;
}
				  
int kbox_dump_painc_info(const char *fmt, ...)
{
	va_list args;
	int num = 0;
	char tmp_buf[PANIC_TMP_BUF_SIZE] = { };

	va_start(args, fmt);

	num = vsnprintf(tmp_buf, sizeof(tmp_buf) - 1, fmt, args);
	if (num >= 0)
		(void)kbox_duplicate_syslog_info(tmp_buf, num);

	va_end(args);

	return num;
}

static void kbox_show_kernel_version(void)
{
	(void)kbox_dump_painc_info
		("\nOS : %s,\nRelease : %s,\nVersion : %s,\n",
		 init_uts_ns.name.sysname,
		 init_uts_ns.name.release,
		 init_uts_ns.name.version);
	(void)kbox_dump_painc_info
		("Machine : %s,\nNodename : %s\n",
		 init_uts_ns.name.machine,
		 init_uts_ns.name.nodename);
}

static void kbox_show_version(void)
{
	(void)kbox_dump_painc_info("\nKBOX_VERSION         : %s\n",
				   KBOX_VERSION);
}

static void kbox_show_time_stamps(void)
{
	struct rtc_time rtc_time_val = { };
	struct timespec64 uptime;

	ktime_get_coarse_real_ts64(&uptime);
	rtc_time64_to_tm(uptime.tv_sec, &rtc_time_val);

	(void)kbox_dump_painc_info
		("Current time         : %04d-%02d-%02d %02d:%02d:%02d\n",
		 rtc_time_val.tm_year + 1900, rtc_time_val.tm_mon + 1,
		 rtc_time_val.tm_mday, rtc_time_val.tm_hour,
		 rtc_time_val.tm_min, rtc_time_val.tm_sec);
}

static void kbox_handle_panic_dump(const char *msg)
{
	if (msg) {
		(void)kbox_dump_painc_info("panic string: %s\n", msg);
	}
}

void kbox_output_syslog_info(void)
{
	unsigned int start_tmp = 0;
	unsigned int end_tmp = 0;
	unsigned int len_tmp = 0;
	unsigned long flags = 0;

	if (unlikely
	    (!g_panic_info_buf || !g_panic_info_buf_tmp))
		return;

	spin_lock_irqsave(&g_panic_buf_lock, flags);
	if (g_panic_info_len == 0) {
		spin_unlock_irqrestore(&g_panic_buf_lock, flags);
		return;
	}

	start_tmp = (g_panic_info_start % SECTION_PANIC_LEN);
	end_tmp = ((g_panic_info_end - 1) % SECTION_PANIC_LEN);
	len_tmp = g_panic_info_len;

	if (start_tmp > end_tmp) {
		memcpy(g_panic_info_buf_tmp,
		       (g_panic_info_buf + start_tmp),
			len_tmp - start_tmp);
		memcpy((g_panic_info_buf_tmp + len_tmp - start_tmp),
		       g_panic_info_buf,
			end_tmp + 1);
	} else {
		memcpy(g_panic_info_buf_tmp,
		       (char *)(g_panic_info_buf + start_tmp),
			len_tmp);
	}

	spin_unlock_irqrestore(&g_panic_buf_lock, flags);

	(void)kbox_write_panic_info(g_panic_info_buf_tmp, len_tmp);
}

void kbox_dump_event(enum kbox_error_type_e type, unsigned long event,
		     const char *msg)
{
	if (!spin_trylock(&g_dump_lock))
		return;

	(void)kbox_dump_painc_info("\n====kbox begin dumping====\n");

	switch (type) {
	case KBOX_REBOOT_EVENT:
	case KBOX_DIE_EVENT:
	case KBOX_PANIC_EVENT:	
		kbox_handle_panic_dump(msg);
		break;
	default:
		break;
	}

	kbox_show_kernel_version();

	kbox_show_version();

	kbox_show_time_stamps();

	(void)kbox_dump_painc_info("\n====kbox end dump====\n");

	kbox_output_syslog_info();
	kbox_output_printk_info();

	spin_unlock(&g_dump_lock);
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
