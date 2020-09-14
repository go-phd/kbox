
#include <linux/types.h>
#include <linux/slab.h>
#include "kbox_collect.h"

struct kbox_collect_info_s *g_collect_info = NULL;


int kbox_collect_process_list(unsigned char **process_list)
{
	unsigned long flags;

	if (!process_list) {
		return -EINVAL;
	}

	spin_lock_irqsave(&g_collect_info->collect_lock, flags);

	if (g_collect_info) {
		if (!g_collect_info->process_list) {
			g_collect_info->process_list = (unsigned char *)kzalloc(1024, GFP_KERNEL);
			if(!g_collect_info->process_list) {
				spin_unlock_irqrestore(&g_collect_info->collect_lock,flags);
				return -ENOMEM;
			}
		}

		//do something
	}

	spin_unlock_irqrestore(&g_collect_info->collect_lock, flags);

	*process_list = g_collect_info->process_list;

	return 0;
}


int kbox_collect_memory_list(unsigned char **memory_list)
{
	unsigned long flags;

	if (!memory_list) {
		return -EINVAL;
	}

	spin_lock_irqsave(&g_collect_info->collect_lock,flags);

	if (g_collect_info) {
		if (!g_collect_info->memory_list) {
			g_collect_info->memory_list = (unsigned char *)kzalloc(1024, GFP_KERNEL);
			if(!g_collect_info->memory_list) {
				spin_unlock_irqrestore(&g_collect_info->collect_lock,flags);
				return -ENOMEM;
			}
		}

		//do something
	}

	spin_unlock_irqrestore(&g_collect_info->collect_lock,flags);

	*memory_list = g_collect_info->memory_list;

	return 0;
}



struct kbox_collect_info_s * kbox_int_collect(void)
{
	g_collect_info = (struct kbox_collect_info_s *)kzalloc(sizeof(struct kbox_collect_info_s), GFP_KERNEL);
	RETURN_VAL_DO_INFO_IF_FAIL(g_collect_info, NULL,
        KBOX_LOG(KLOG_ERROR, "g_collect_info kzalloc failed! \n"););

	spin_lock_init(&g_collect_info->collect_lock);


	return g_collect_info;
}


int kbox_cleanup_collect(void)
{
	if(g_collect_info) {
        kfree(g_collect_info);
        g_collect_info = NULL;
    }

	return 0;
}



