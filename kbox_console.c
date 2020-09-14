
#include "kbox_console.h"

static int g_event_flag = 0;

void kbox_enable_console_write(void)
{
	g_event_flag = 1;
}


static void kbox_console_write(struct console *co, const char *buf, unsigned count)
{
	if (g_event_flag) {
		//kbox_write_to_syscom(buf, count);
		//KBOX_LOG(KLOG_ERROR, "buf = %s \n", buf);
	}
}


static struct console kbox_console = {
	.name    = "KboxConsole",
	.write   = kbox_console_write,
	.flags   = CON_ENABLED | CON_PRINTBUFFER,
};


int kbox_init_console(void)
{
	register_console(&kbox_console);

	return 0;
}

void kbox_cleanup_console(void)
{
	unregister_console(&kbox_console);
}



