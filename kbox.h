
#ifndef _KBOX_H
#define _KBOX_H

#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/ioctl.h>


#define KBOX_IOC_MAGIC 'k'
#define KBOX_REGISTER_TYPE _IOW(KBOX_IOC_MAGIC, 100, unsigned long)


enum {
    KLOG_DEBUG = 0,
    KLOG_ERROR = 1,
};


extern int debug_level;
#define KBOX_LOG(level, fmt, args...) do {\
    if (level >= debug_level) { \
        printk(KERN_ALERT "KBOX: %s(), %s, %d, " fmt, __func__, __FILE__, __LINE__, ##args);\
    }\
}while(0)


#define RETURN_VAL_DO_INFO_IF_FAIL(expr, val, info) do {\
    if (unlikely(!(expr))) { \
        info;\
        return val;\
    }\
}while(0)


#define RETURN_DO_INFO_IF_FAIL(expr, info) do {\
    if (unlikely(!(expr))) { \
        info;\
        return;\
    }\
}while(0)


#define DO_INFO_IF_EXPR_LIKELY(expr, info) do {\
    if (likely(expr)) { \
        info;\
    }\
}while(0)


#define DO_INFO_IF_EXPR_UNLIKELY(expr, info) do {\
    if (unlikely(expr)) { \
        info;\
    }\
}while(0)


#define RETURN_VAL_DO_INFO_IF_IS_ERR(expr, val, info) do {\
    if (IS_ERR(expr)) { \
        info;\
        return val;\
    }\
}while(0)


#define DO_INFO_IF_EXPR_IS_ERR(expr, info) do {\
    if (IS_ERR(expr)) { \
        info;\
    }\
}while(0)


struct kbox_collect_info_s {
	spinlock_t collect_lock;
	unsigned char *process_list;
	unsigned char *memory_list;
};


struct kbox_descriptor_s {
    //struct chris_dev_s *chris_dev;
    //spinlock_t msg_mem_lock;
    
    //struct proc_dir_entry *proc_root;
	//struct edma_host_s edma_host;
	struct kbox_collect_info_s *collect_info;
};


#endif


