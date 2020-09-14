;
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>

#include "kbox_ipmi.h"

static ipmi_user_t ipmi_user;
static int ipmi_ifnum = -1;


/* Stuff from the get device id command. */
static unsigned int mfg_id;
static unsigned int prod_id;
static unsigned char capabilities;
static unsigned char ipmi_version;

/*
 * We use our own messages for this operation, we don't let the system
 * allocate them, since we may be in a panic situation.  The whole
 * thing is single-threaded, anyway, so multiple messages are not
 * required.
 */
static atomic_t dummy_count = ATOMIC_INIT(0);
static void dummy_smi_free(struct ipmi_smi_msg *msg)
{
	atomic_dec(&dummy_count);
}
static void dummy_recv_free(struct ipmi_recv_msg *msg)
{
	atomic_dec(&dummy_count);
}
static struct ipmi_smi_msg halt_smi_msg = {
	.done = dummy_smi_free
};
static struct ipmi_recv_msg halt_recv_msg = {
	.done = dummy_recv_free
};


/*
 * Code to send a message and wait for the response.
 */
static void kbox_receive_ipmi_handler(struct ipmi_recv_msg *recv_msg, void *handler_data)
{
	struct completion *comp = recv_msg->user_msg_data;

	if (comp)
		complete(comp);
}


static struct ipmi_user_hndl kboc_ipmi_handler = {
	.ipmi_recv_hndl = kbox_receive_ipmi_handler
};


static int kbox_ipmi_request_wait_for_response(ipmi_user_t            user,
					  struct ipmi_addr       *addr,
					  struct kernel_ipmi_msg *send_msg)
{
	int               rv;
	struct completion comp;
	static unsigned long msgid = 0;

	init_completion(&comp);

	rv = ipmi_request_supply_msgs(user, addr, msgid, send_msg, &comp,
				      &halt_smi_msg, &halt_recv_msg, 0);
	if (rv)
		return rv;

	wait_for_completion(&comp);

	msgid++;

	return halt_recv_msg.msg.data[0];
}
					  
					  
int kbox_ipmi_send_request(unsigned char *data, unsigned short data_len)
{
  int ret = 0;
  struct ipmi_system_interface_addr smi_addr = {};
  struct kernel_ipmi_msg			send_msg = {};

  unsigned char send_data[] = {0x63, 0x38, 0x02, 0x38, 0x6d, 0x43, 0xc6, 0x21, 0x00, 0x03, 0x20, 0xff, 0x6f, 0x00, 0x00, 0x00}; 

  smi_addr.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
  smi_addr.channel = IPMI_BMC_CHANNEL;
  smi_addr.lun = 0;

  //send_msg.netfn = IPMI_NETFN_APP_REQUEST;
  //send_msg.cmd = IPMI_GET_DEVICE_ID_CMD;
  //send_msg.data = data;
  //send_msg.data_len = data_len;

  send_msg.netfn = IPMI_NETFN_STORAGE_REQUEST;
  send_msg.cmd = IPMI_ADD_SEL_ENTRY_CMD;
  send_msg.data = send_data;
  send_msg.data_len = sizeof(send_data);
  
  ret = kbox_ipmi_request_wait_for_response(ipmi_user,
					  (struct ipmi_addr *) &smi_addr,
					  &send_msg);
  RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret,
	  KBOX_LOG(KLOG_ERROR, "Unable to send IPMI get device id info, IPMI error 0x%x\n", ret));

  {
	  int i = 0;
	  int count = 0;
	  char str_buf[1024] = {};

	  printk(KERN_ALERT "ipmi recv data: %d\n", i);
	  //count += sprintf(str_buf, "ipmi recv:");
	  for (i = 0; i < halt_recv_msg.msg.data_len; i++) {
		  printk(KERN_ALERT "0x%x ", halt_recv_msg.msg.data[i]);
		  //count += sprintf(str_buf, "0x%x ", halt_recv_msg.msg.data[i]);
	  }
	  //count += sprintf(str_buf, "\n");

	  printk(KERN_ALERT "\n");
	  //kbox_write_to_syscom(str_buf, count);		  
  }   

  return 0;
}


static void kbox_ipmi_new_smi(int if_num, struct device *device)
{
	struct ipmi_system_interface_addr smi_addr;
	struct kernel_ipmi_msg            send_msg;
	int                               rv;

	if ((ipmi_ifnum >= 0) && (ipmi_ifnum != if_num))
		return;

	rv = ipmi_create_user(if_num, &kboc_ipmi_handler, NULL,
			      &ipmi_user);
	if (rv) {
		KBOX_LOG(KLOG_ERROR, "could not create IPMI user, error %d\n",
		       rv);
		return;
	}

	ipmi_ifnum = if_num;

	/*
	 * Do a get device ide and store some results, since this is
	 * used by several functions.
	 */
	smi_addr.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
	smi_addr.channel = IPMI_BMC_CHANNEL;
	smi_addr.lun = 0;

	send_msg.netfn = IPMI_NETFN_APP_REQUEST;
	send_msg.cmd = IPMI_GET_DEVICE_ID_CMD;
	send_msg.data = NULL;
	send_msg.data_len = 0;
	rv = kbox_ipmi_request_wait_for_response(ipmi_user,
					    (struct ipmi_addr *) &smi_addr,
					    &send_msg);
	if (rv) {
		KBOX_LOG(KLOG_ERROR, "Unable to send IPMI get device id info,"
		       " IPMI error 0x%x\n", rv);
		goto out_err;
	}

	if (halt_recv_msg.msg.data_len < 12) {
		KBOX_LOG(KLOG_ERROR, "(chassis) IPMI get device id info too,"
		       " short, was %d bytes, needed %d bytes\n",
		       halt_recv_msg.msg.data_len, 12);
		goto out_err;
	}

	mfg_id = (halt_recv_msg.msg.data[7]
		  | (halt_recv_msg.msg.data[8] << 8)
		  | (halt_recv_msg.msg.data[9] << 16));
	prod_id = (halt_recv_msg.msg.data[10]
		   | (halt_recv_msg.msg.data[11] << 8));
	capabilities = halt_recv_msg.msg.data[6];
	ipmi_version = halt_recv_msg.msg.data[5];

	KBOX_LOG(KLOG_ERROR, "mfg_id = 0x%x, prod_id = 0x%x, capabilities = 0x%x, ipmi_version = 0x%x\n", mfg_id, prod_id, capabilities, ipmi_version);

	//kbox_ipmi_send_request(NULL, 0);

	return;

 out_err:
	KBOX_LOG(KLOG_ERROR, "Unable to find a ipmi function that"
	       " will work, giving up\n");
	ipmi_destroy_user(ipmi_user);
}



static void kbox_ipmi_smi_gone(int if_num)
{
	int ret = 0;

	if (ipmi_ifnum != if_num)
		return;

	ret = ipmi_destroy_user(ipmi_user);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "ipmi_destroy_user failed! ret = %d\n", ret));
}


static struct ipmi_smi_watcher kbxo_smi_watcher = {
	.owner    = THIS_MODULE,
	.new_smi  = kbox_ipmi_new_smi,
	.smi_gone = kbox_ipmi_smi_gone
};


int kbox_init_ipmi(void)
{
	int ret = 0;
	
	ret = ipmi_smi_watcher_register(&kbxo_smi_watcher);
	RETURN_VAL_DO_INFO_IF_FAIL(!ret, ret,
        KBOX_LOG(KLOG_ERROR, "ipmi_smi_watcher_register failed! ret = %d\n", ret));

	return 0;
}

void kbox_cleanup_ipmi(void)
{
	int ret = 0;
	
	ret = ipmi_smi_watcher_unregister(&kbxo_smi_watcher);
	DO_INFO_IF_EXPR_UNLIKELY(ret, 
        KBOX_LOG(KLOG_ERROR, "ipmi_smi_watcher_unregister failed! ret = %d\n", ret));
}



