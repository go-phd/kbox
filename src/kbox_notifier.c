#include <linux/kernel.h>
#include <linux/kdebug.h>
#include <linux/notifier.h>
#include <linux/power_supply.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>

//#include <asm/mce.h>
#include <phd/kbox.h>

#include "kbox_notifier.h"
#include "kbox_collect.h"
#include "kbox_console.h"
#include "kbox_netlink.h"
#include "kbox_dump.h"

struct kbox_notifier_s *kbox_notifier = NULL;

/*
static int kbox_power_supply_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	int ret = 0;
	char str_buf[512] = {};
	int count = 0;

	UNUSED(this);

	count = snprintf(str_buf, sizeof(str_buf), "power_supply! event : 0x%lx, msg : %s\n", event, (char *)ptr);

	// 发送消息到用户态进程
	ret = kbox_broadcast(KBOX_NLGRP_SYSTEM_EVENT, KBOX_NL_CMD_POWER_SUPPLY, str_buf, strlen(str_buf), GFP_ATOMIC);
	DO_INFO_IF_EXPR_UNLIKELY(ret,
        KBOX_LOG(KLOG_ERROR, "kbox_broadcast fail, ret = %d\n", ret););

	// dump 数据到 保留内存
	kbox_dump_event(KBOX_POEWER_SUPPLY_EVENT, event, (const char *)ptr);
	
	return NOTIFY_OK;
}


static struct notifier_block kbox_power_supply_block = {
	.notifier_call	= kbox_power_supply_event,
	.priority	= INT_MAX,
};
*/

static int kbox_reboot_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	//int ret = 0;
	char str_buf[512] = {};
	int count = 0;

	UNUSED(this);

	count = snprintf(str_buf, sizeof(str_buf), "reboot! event : 0x%lx, msg : %s\n", event, (char *)ptr);

	// dump 数据到 保留内存
	kbox_dump_event(KBOX_REBOOT_EVENT, event, (const char *)ptr);
	
	return NOTIFY_OK;
}


static struct notifier_block kbox_reboot_block = {
	.notifier_call	= kbox_reboot_event,
	.priority	= INT_MAX,
};


static int kbox_die_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	//int ret = 0;
	char str_buf[512] = {};
	int count = 0;
	struct die_args *die = (struct die_args *)ptr;

	UNUSED(this);

	count += snprintf(str_buf + count, sizeof(str_buf) - count, "die! event : 0x%lx\n", event);
	
	if (ptr) {
		int i = 0;
		char *p = (char *)&(die->regs->user_regs);
		
		count += snprintf(str_buf + count, sizeof(str_buf) - count, "die_args : \n");
		count += snprintf(str_buf + count, sizeof(str_buf) - count, "str : %s, err : 0x%lx, trapnr : 0x%x, signr : 0x%x\n", die->str, die->err, die->trapnr, die->signr);
		count += snprintf(str_buf + count, sizeof(str_buf) - count, "die->regs : \n");
		count += snprintf(str_buf + count, sizeof(str_buf) - count, "orig_x0 : 0x%llx, syscallno : 0x%llx, orig_addr_limit : 0x%llx\n", die->regs->orig_x0, die->regs->syscallno, die->regs->orig_addr_limit);
		count += snprintf(str_buf + count, sizeof(str_buf) - count, "user_regs : \n");
		count += snprintf(str_buf + count, sizeof(str_buf) - count, "user_regs : \n");
		for (i = 0; i < sizeof(struct user_pt_regs); i++) {
			count += snprintf(str_buf + count, sizeof(str_buf) - count, "0x%x \n", p[i]);
		}
		count += snprintf(str_buf + count, sizeof(str_buf) - count, "\n");
	}

	// dump 数据到 保留内存
	kbox_dump_event(KBOX_DIE_EVENT, event, (const char *)str_buf);
	
	return NOTIFY_OK;
}


static struct notifier_block kbox_die_block = {
	.notifier_call	= kbox_die_event,
	.priority	= INT_MAX,
};


int kbox_panic_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	//int ret = 0;
	char str_buf[512] = {};
	int count = 0;

	UNUSED(this);

	count = snprintf(str_buf, sizeof(str_buf), "panic! event : 0x%lx, msg : %s\n", event, (char *)ptr);

	// dump 数据到 保留内存
	kbox_dump_event(KBOX_PANIC_EVENT, event, (const char *)ptr);
	
	return NOTIFY_DONE;
}


static struct notifier_block kbox_panic_block = {
	.notifier_call	= kbox_panic_event,
	.priority	= INT_MAX,
};


irqreturn_t kbox_isr_sb2_dbg(int irq, void *pdev)
{
	char str_buf[512] = {};
	int count = 0;
	//int ret = 0;

	//KBOX_LOG(KLOG_ERROR, "irq = %d, pdev = %p\n", irq, pdev);
	UNUSED(pdev);

	count = snprintf(str_buf, sizeof(str_buf), "isr_sb2_dbg, may be power input loss");

	// 发送消息到用户态进程
	//ret = kbox_broadcast(KBOX_NLGRP_DEVICE_EVENT, KBOX_NL_CMD_SB2, str_buf, count, GFP_ATOMIC);
	//DO_INFO_IF_EXPR_UNLIKELY(ret,
    //    KBOX_LOG(KLOG_ERROR, "kbox_broadcast fail, ret = %d\n", ret););

	// dump 数据到 保留内存
	kbox_dump_event(KBOX_POEWER_SUPPLY_EVENT, 0, (const char *)str_buf);

	return IRQ_NONE;
}


int kbox_init_notifier(void)
{
	int ret = 0;
	kbox_notifier = kzalloc(sizeof(struct kbox_notifier_s), GFP_KERNEL);
	if (IS_ERR(kbox_notifier) || !kbox_notifier) {
		KBOX_LOG(KLOG_ERROR, "kmalloc g_printk_info_buf fail!\n");
		return -ENOMEM;
	}

	ret = kbox_init_dump();
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret,
			KBOX_LOG(KLOG_ERROR, "kbox_init_dump failed! ret = %d\n", ret););
	
	//ret = power_supply_reg_notifier(&kbox_power_supply_block);
	//DO_INFO_IF_EXPR_UNLIKELY(ret,
    //    KBOX_LOG(KLOG_ERROR, "power_supply_reg_notifier failed! ret = %d\n", ret); goto fail;);
	
	ret = register_reboot_notifier(&kbox_reboot_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "register_reboot_notifier failed! ret = %d\n", ret); goto fail;);

	ret = register_die_notifier(&kbox_die_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "register_die_notifier failed! ret = %d\n", ret); goto fail;);

	ret = atomic_notifier_chain_register(&panic_notifier_list, &kbox_panic_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "atomic_notifier_chain_register panic_notifier_list failed! ret = %d\n", ret); goto fail;);

	// 监控 sb2_dbg 中断
	//kbox_notifier->sb2_dbg_irq = 26;
	//ret = request_irq(kbox_notifier->sb2_dbg_irq, kbox_isr_sb2_dbg, IRQF_SHARED, "kbox_sb2_dbg", kbox_notifier);
	//DO_INFO_IF_EXPR_UNLIKELY(ret,
    //    KBOX_LOG(KLOG_ERROR, "request_irq failed! ret = %d\n", ret); goto fail;);

	return 0;

fail:
	kbox_cleanup_notifier();

	return ret;
}

void kbox_cleanup_notifier(void)
{
	int ret = 0;

	if (kbox_notifier) {
		//free_irq(kbox_notifier->sb2_dbg_irq, kbox_notifier);
		kfree(kbox_notifier);
		kbox_notifier = NULL;
	}
	
	//power_supply_unreg_notifier(&kbox_power_supply_block);
	
	ret = unregister_reboot_notifier(&kbox_reboot_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "unregister_reboot_notifier failed! ret = %d\n", ret););

	ret = unregister_die_notifier(&kbox_die_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "unregister_die_notifier failed! ret = %d\n", ret););

	ret = atomic_notifier_chain_unregister(&panic_notifier_list, &kbox_panic_block);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "atomic_notifier_chain_unregister panic_notifier_list failed! ret = %d\n", ret););

	kbox_cleanup_dump();
}



