
#include <linux/fs.h>		/* everything... */
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <asm/ioctls.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <phd/kbox.h>

#include "kbox_cdev.h"
#include "kbox_ram_image.h"
#include "kbox_ram_op.h"
#include "kbox_dump.h"
#include "kbox_notifier.h"
#include "kbox_console.h"


#define KBOX_DEVICE_NAME "kbox"
#define KBOX_DEVICE_MINOR 255

static struct kbox_dev_s *g_kbox_dev;

static ssize_t kbox_read(struct file *filp, char __user *data, size_t count,
			 loff_t *ppos)
{
	int read_len = 0;

	if (!filp || !data || !ppos) {
		KBOX_LOG(KLOG_ERROR, "input NULL point!\n");
		return -EFAULT;
	}

	// test
	kbox_panic_event(NULL, 1, "test panic");
	kbox_console_debug_print();

	read_len = kbox_read_op((long long)(*ppos),
				count,
				data);
	if (read_len < 0)
		return -EFAULT;

	*ppos = read_len;

	return read_len;
}


static ssize_t kbox_write(struct file *filp, const char __user *data,
		   size_t count, loff_t *ppos)
{

	char *p = NULL;

	// 构造无效地址访问异常宕机
	*p = 1;

	return 0;
}


static int kbox_ioctl_verify_cmd(unsigned int cmd, unsigned long arg)
{
	if (arg == 0 || (_IOC_TYPE(cmd) != KBOX_IOC_MAGIC))
		return -EFAULT;

	if (_IOC_NR(cmd) > KBOX_IOC_MAXNR)
		return -EFAULT;

	if (!capable(CAP_SYS_ADMIN)) {
		KBOX_LOG(KLOG_ERROR, "permit error\n");
		return -EFAULT;
	}

	return 0;
}

static long kbox_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	UNUSED(filp);

	if (kbox_ioctl_verify_cmd(cmd, arg) < 0) {
		return -EFAULT;
	}
		
	switch (cmd) {
	case GET_KBOX_TOTAL_LEN:
		break;

	case CLEAR_KBOX_REGION_ALL:
		break;

	case KBOX_ISM_SET_CTRL_PID:
		{
			struct kbox_ioctl_lsm_set_s lsmset = {};
			if (copy_from_user((void *)&lsmset, (void __user *)arg, sizeof(struct kbox_ioctl_lsm_set_s))) {
				KBOX_LOG(KLOG_ERROR, "fail to copy_to_user!\n");
				return -EINVAL;
			}

			KBOX_LOG(KLOG_DEBUG, "type = %d, service_name = %s\n", lsmset.type, lsmset.service_name);
			
			return add_ctrl_current_pid((enum phdlsm_type_e)lsmset.type, lsmset.service_name);
		}
		break;
	case KBOX_ISM_SET_CTRL_FILE:
		{
		struct kbox_ioctl_lsm_set_s lsmset = {};
			
			if (copy_from_user((void *)&lsmset, (void __user *)arg, sizeof(struct kbox_ioctl_lsm_set_s))) {
				KBOX_LOG(KLOG_ERROR, "fail to copy_to_user!\n");
				return -EINVAL;
			}

			KBOX_LOG(KLOG_DEBUG, "type = %d, file_name = %s\n", lsmset.type, lsmset.file_name);
			
			return add_ctrl_file((enum phdlsm_type_e)lsmset.type, lsmset.file_name);
		}
		break;
	default:
		KBOX_LOG(KLOG_DEBUG, "cmd fail, 0x%x\n", cmd);
		return -EINVAL;
	}

	return 0;
}

static int kbox_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long kbox_section_phy_addr = kbox_get_section_phy_addr(KBOX_SECTION_USER);
	unsigned long kbox_section_len = kbox_get_section_len(KBOX_SECTION_USER);
	unsigned long offset = 0;
	unsigned long length = 0;
	unsigned long vm_size = 0;
	unsigned long remap_phy_addr = kbox_section_phy_addr;
		
	int ret = 0;

	if (!filp || !vma) {
		KBOX_LOG(KLOG_ERROR, "input NULL point!\n");
		return -EFAULT;
	}

	offset = vma->vm_pgoff << PAGE_SHIFT;
	vm_size = vma->vm_end - vma->vm_start;

	if (offset >= kbox_section_len) {
		KBOX_LOG(KLOG_ERROR, "vma offset is invalid!\n");
		return -ESPIPE;
	}

	if (vma->vm_flags & VM_LOCKED) {
		KBOX_LOG(KLOG_ERROR, "vma is locked!\n");
		return -EPERM;
	}
	
	length = kbox_section_len - offset;
	remap_phy_addr = kbox_section_phy_addr + offset;
	
	KBOX_LOG(KLOG_DEBUG, "offset = %ld, vm_size = %ld, length = %ld\n", offset, vm_size, length);
	KBOX_LOG(KLOG_DEBUG, "remap_phy_addr= 0x%lx\n", remap_phy_addr);
	
	if (vm_size > length) {
		KBOX_LOG(KLOG_ERROR, "vm_size is invalid!\n");
		return -ENOSPC;
	}

	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;	//该内存区不能被换出，老版本内核为 VM_RESERVED
	vma->vm_flags |= VM_IO;

	ret = remap_pfn_range(vma,
			      vma->vm_start,
			      (unsigned long)(remap_phy_addr >>
					      PAGE_SHIFT), vm_size,
			      vma->vm_page_prot);
	if (ret) {
		KBOX_LOG(KLOG_ERROR, "remap_pfn_range failed! ret = %d\n", ret);
		return -EAGAIN;
	}

	return 0;
}

static int kbox_open(struct inode *pinode, struct file *filp)
{
	UNUSED(pinode);

	if ((g_kbox_dev) && (!atomic_dec_and_test(&g_kbox_dev->au_count))) {
		atomic_inc(&g_kbox_dev->au_count);
		KBOX_LOG(KLOG_ERROR, "EBUSY\n");
		return -EBUSY;
	}

	filp->private_data = (void *)g_kbox_dev;

	return 0;
}

int kbox_release(struct inode *pinode, struct file *filp)
{
	struct kbox_dev_s *kbox_dev = (struct kbox_dev_s *)filp->private_data;

	UNUSED(pinode);

	if (kbox_dev)
		atomic_inc(&kbox_dev->au_count);

	return 0;
}

const struct file_operations kbox_fops = {
	.owner = THIS_MODULE,
	.read = kbox_read,
	.write = kbox_write,
	.unlocked_ioctl = kbox_ioctl,
	.mmap = kbox_mmap,
	.open = kbox_open,
	.release = kbox_release,
};

static struct miscdevice kbox_device = {
	KBOX_DEVICE_MINOR,
	KBOX_DEVICE_NAME,
	&kbox_fops,
};

int kbox_init_cdev(void)
{
	int ret = 0;

	g_kbox_dev = kmalloc(sizeof(struct kbox_dev_s), GFP_KERNEL);
	if (!g_kbox_dev)
		return -ENOMEM;

	ret = misc_register(&kbox_device);
	if (ret)
		goto fail;

	atomic_set(&g_kbox_dev->au_count, 1);

	return ret;

fail:
	kfree(g_kbox_dev);
	g_kbox_dev = NULL;

	return ret;
}

void kbox_cleanup_cdev(void)
{
	if (!g_kbox_dev)
		return;

	misc_deregister(&kbox_device);

	kfree(g_kbox_dev);
	g_kbox_dev = NULL;
}

