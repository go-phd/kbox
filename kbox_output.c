#include <linux/fs.h>
#include <asm/uaccess.h>

#include "kbox_ipmi.h"
#include "kbox_pcie.h"
#include "kbox_output.h"


void kbox_write_to_syscom(char *str, unsigned count)
{
	int ret = 0;
	mm_segment_t fs;
	loff_t pos;
	struct file *filp;
	//unsigned count = strlen(str);
	
	fs = get_fs();
	set_fs(KERNEL_DS);
	
	filp = filp_open("/dev/ttyS0", O_WRONLY, 0666);
	if (IS_ERR(filp)) {
		KBOX_LOG(KLOG_ERROR, "filp_open failed! \n");
	}

	if (filp->f_mode & FMODE_WRITE){
		ssize_t (*write)(struct file *, const char *, size_t, loff_t *);
		if (filp->f_op && (filp->f_op->write) != NULL){
			write = filp->f_op->write;
			ret = write(filp, (const char *)str, count, &filp->f_pos);
			//KBOX_LOG(KLOG_ERROR, "write ret = %d \n", ret);
		}
	}
	
	//ret = vfs_write(filp, str, count, &pos);
	//KBOX_LOG(KLOG_ERROR, "vfs_write ret = %d \n", ret);
	//vfs_fsync(filp, 0);

	filp_close(filp, NULL); 
	set_fs(fs);
}


int kbox_output(unsigned char *data, unsigned short data_len)
{
	if (kbox_pcie_get_proeb_flag()) {
		return kbox_pcie_send_request(data, data_len);
	} else {
		return kbox_ipmi_send_request(data, data_len);
	}
}

int kbox_init_output(void)
{
	int ret = 0;

	ret = kbox_init_pcie();
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "kbox_init_pcie failed! ret = %d\n", ret));

	if (0 == kbox_pcie_get_proeb_flag()) {
		ret = kbox_init_ipmi();
		RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret,
	        KBOX_LOG(KLOG_ERROR, "kbox_init_ipmi_output failed! ret = %d\n", ret));
	}

	return ret;
}


void kbox_cleanup_output(void)
{
	if (kbox_pcie_get_proeb_flag()) {
		kbox_cleanup_pcie();
	} else {
		kbox_cleanup_ipmi();
	}
}



