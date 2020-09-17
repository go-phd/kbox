#include <linux/fs.h>
#include <asm/uaccess.h>
#include <net/netlink.h>

#include "kbox_netlink.h"


struct sock *nlsk = NULL;
extern struct net init_net;

static int kbox_unicast(u32 pid, void *msg, u32 seq, size_t size, gfp_t gfp_mask)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    // 创建sk_buff 空间 包含netlink消息头部空间
    nl_skb = nlmsg_new(size, gfp_mask);
    RETURN_VAL_DO_INFO_IF_FAIL(nl_skb, -ENOMEM,
        KBOX_LOG(KLOG_ERROR, "nlmsg_new fail\n"););

    // 设置netlink消息头部
    // portid 内核主动发往用户态的消息，该字段设置为 0
    // seq 内核收到该消息后对提取其中的序列号，发送给用户程序回应消息里设置同样的序列号，确认机制
    // type 消息的类型，是数据 or 控制消息
    // payload 整个消息的长度，按字节计算。包括了Netlink消息头本身
    // flags 附加在消息上的额外说明信息，比如 NLM_F_MULTI
    nlh = nlmsg_put(nl_skb, 0, seq, NLMSG_NOOP, size, 0);
    RETURN_VAL_DO_INFO_IF_FAIL(nlh, -EMSGSIZE,
        KBOX_LOG(KLOG_ERROR, "nlmsg_put fail\n"); nlmsg_free(nl_skb););

	memcpy(nlmsg_data(nlh), msg, size);

	NETLINK_CB(nl_skb).portid = 0;		/* from kernel */
	NETLINK_CB(nl_skb).dst_group = 0;  	/* unicast */
	
    return nlmsg_unicast(nlsk, nl_skb, pid);
}


static void kbox_process_netlink_rcv_msg(struct kbox_nl_msg_hdr_s *msg, u32 pid, u32 seq) 
{
	KBOX_LOG(KLOG_ERROR, "kbox_process_netlink_rcv_msg, pid = %x, seq = %d\n", pid, seq);
	KBOX_LOG(KLOG_ERROR, "kbox_process_netlink_rcv_msg, %d, %d, %s\n", msg->cmd, msg->datalen, msg->data);

	switch (msg->cmd) {
	case 1:
		// do something 
		KBOX_LOG(KLOG_ERROR, "do something, cmd = %d\n", msg->cmd);
		break;
	case 2:
		{
			int ret = 0;
			struct kbox_nl_msg_hdr_s hdr = {};
			
			hdr.cmd = msg->cmd;
			hdr.dir = DIR_KERNEL_2_USER;
			hdr.datalen = 0;
				
			ret = kbox_unicast(pid, (void*)&hdr, seq, sizeof(struct kbox_nl_msg_hdr_s), GFP_KERNEL);
			RETURN_DO_INFO_IF_FAIL(!ret,
	        	KBOX_LOG(KLOG_ERROR, "kbox_unicast fail, ret = %d\n", ret););
		}
		break;
	default:
		KBOX_LOG(KLOG_ERROR, "err cmd, cmd = %d\n", msg->cmd);
		break;
	}
}

static void kbox_netlink_rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;

    if(skb->len >= nlmsg_total_size(0))
    {
        nlh = nlmsg_hdr(skb);
        umsg = NLMSG_DATA(nlh);
        if(umsg)
        {	
			kbox_process_netlink_rcv_msg((struct kbox_nl_msg_hdr_s *)umsg, nlh->nlmsg_pid, nlh->nlmsg_seq);
        }
    }
}


int kbox_broadcast(u32 group, int cmd, void *msg, size_t size, gfp_t gfp_mask)
{
	int len = SIZE_OF_NL_EVNET_HDR + size;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	struct kbox_nl_event_hdr_s *event = NULL;

	int cnt = netlink_has_listeners(nlsk, group);
	RETURN_VAL_DO_INFO_IF_FAIL(cnt, 0,
        KBOX_LOG(KLOG_ERROR, "netlink have no listeners, cnt = %d\n", cnt););
		
	skb = nlmsg_new(len, gfp_mask);
	RETURN_VAL_DO_INFO_IF_FAIL(skb, -ENOMEM,
        KBOX_LOG(KLOG_ERROR, "nlmsg_new fail\n"););

	// 设置netlink消息头部
	// portid 内核主动发往用户态的消息，该字段设置为 0
	// seq 广播设置为0
	// type 消息的类型，是数据 or 控制消息
	// payload 整个消息的长度，按字节计算。包括了Netlink消息头本身
	// flags 附加在消息上的额外说明信息，比如 NLM_F_MULTI
	nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, len, 0);
	RETURN_VAL_DO_INFO_IF_FAIL(nlh, -EMSGSIZE,
        KBOX_LOG(KLOG_ERROR, "nlmsg_put fail\n"); nlmsg_free(skb););

	event = nlmsg_data(nlh);
	event->event_type = group;
	event->cmd = cmd;
	event->datalen = size;
	memcpy(event->data, msg, size);

	return nlmsg_multicast(nlsk, skb, 0, group, gfp_mask);
}


struct netlink_kernel_cfg cfg = { 
	.groups = KBOX_NLGRP_MAX,
	.input = kbox_netlink_rcv_msg,
};  

int kbox_init_netlink(void)
{
	struct sock *sk = NULL;
	
    sk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_SSF, &cfg);
	RETURN_VAL_DO_INFO_IF_FAIL(sk, -EFAULT,
        KBOX_LOG(KLOG_ERROR, "netlink_kernel_create failed!\n"););

	nlsk = sk;

#ifdef TESTBC
	timer_setup(&timer, test_timeout, 0);
	(void)mod_timer(&timer, jiffies_64 + HZ);
#endif

	return 0;
}


void kbox_cleanup_netlink(void)
{
#ifdef TESTBC
	(void)del_timer_sync(&timer);
#endif

	if (nlsk){
        netlink_kernel_release(nlsk); /* release ..*/
        nlsk = NULL;
    }   
}



