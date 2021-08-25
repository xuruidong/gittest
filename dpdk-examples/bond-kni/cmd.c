/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include <stdint.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <inttypes.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_arp.h>
#include <rte_spinlock.h>

#include <cmdline_rdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_num.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_ipaddr.h>
#include <cmdline_parse_etheraddr.h>
#include <cmdline_socket.h>
#include <cmdline.h>

#include "main.h"

#include <rte_devargs.h>


#include "rte_byteorder.h"
#include "rte_cpuflags.h"
#include "rte_eth_bond.h"

#define RTE_LOGTYPE_DCB RTE_LOGTYPE_USER1

#define NB_MBUF   (1024*8)

#define MAX_PKT_BURST 32
#define BURST_TX_DRAIN_US 100      /* TX drain every ~100us */
#define BURST_RX_INTERVAL_NS (10) /* RX poll interval ~100ns */

/*
 * RX and TX Prefetch, Host, and Write-back threshold values should be
 * carefully set for optimal performance. Consult the network
 * controller's datasheet and supporting DPDK documentation for guidance
 * on how these parameters should be set.
 */
#define RX_PTHRESH 8 /**< Default values of RX prefetch threshold reg. */
#define RX_HTHRESH 8 /**< Default values of RX host threshold reg. */
#define RX_WTHRESH 4 /**< Default values of RX write-back threshold reg. */
#define RX_FTHRESH (MAX_PKT_BURST * 2)/**< Default values of RX free threshold reg. */

/*
 * These default values are optimized for use with the Intel(R) 82599 10 GbE
 * Controller and the DPDK ixgbe PMD. Consider using other values for other
 * network controllers and/or network drivers.
 */
#define TX_PTHRESH 36 /**< Default values of TX prefetch threshold reg. */
#define TX_HTHRESH 0  /**< Default values of TX host threshold reg. */
#define TX_WTHRESH 0  /**< Default values of TX write-back threshold reg. */

/*
 * Configurable number of RX/TX ring descriptors
 */
#define RTE_RX_DESC_DEFAULT 1024
#define RTE_TX_DESC_DEFAULT 1024

#define BOND_IP_1	10
#define BOND_IP_2	0
#define BOND_IP_3	120
#define BOND_IP_4	17

/* not defined under linux */
#ifndef NIPQUAD
#define NIPQUAD_FMT "%u.%u.%u.%u"
#endif

#define MAX_PORTS	4
#define PRINT_MAC(addr)		printf("%02"PRIx8":%02"PRIx8":%02"PRIx8 \
		":%02"PRIx8":%02"PRIx8":%02"PRIx8,	\
		addr.addr_bytes[0], addr.addr_bytes[1], addr.addr_bytes[2], \
		addr.addr_bytes[3], addr.addr_bytes[4], addr.addr_bytes[5])

extern uint16_t slaves[RTE_MAX_ETHPORTS];
extern uint16_t slaves_count;

extern uint16_t BOND_PORT;

extern struct rte_mempool *mbuf_pool;



extern struct global_flag_stru_t *global_flag_stru_p;



struct cmd_obj_send_result {
	cmdline_fixed_string_t action;
	cmdline_ipaddr_t ip;
};
static inline void get_string(struct cmd_obj_send_result *res, char *buf, uint8_t size)
{
	snprintf(buf, size, NIPQUAD_FMT,
		((unsigned)((unsigned char *)&(res->ip.addr.ipv4))[0]),
		((unsigned)((unsigned char *)&(res->ip.addr.ipv4))[1]),
		((unsigned)((unsigned char *)&(res->ip.addr.ipv4))[2]),
		((unsigned)((unsigned char *)&(res->ip.addr.ipv4))[3])
		);
}
static void cmd_obj_send_parsed(void *parsed_result,
		__rte_unused struct cmdline *cl,
			       __rte_unused void *data)
{

	struct cmd_obj_send_result *res = parsed_result;
	char ip_str[INET6_ADDRSTRLEN];

	struct rte_ether_addr bond_mac_addr;
	struct rte_mbuf *created_pkt;
	struct rte_ether_hdr *eth_hdr;
	struct rte_arp_hdr *arp_hdr;

	uint32_t bond_ip;
	size_t pkt_size;
	int ret;

	if (res->ip.family == AF_INET)
		get_string(res, ip_str, INET_ADDRSTRLEN);
	else
		cmdline_printf(cl, "Wrong IP format. Only IPv4 is supported\n");

	bond_ip = BOND_IP_1 | (BOND_IP_2 << 8) |
				(BOND_IP_3 << 16) | (BOND_IP_4 << 24);

	ret = rte_eth_macaddr_get(BOND_PORT, &bond_mac_addr);
	if (ret != 0) {
		cmdline_printf(cl,
			       "Failed to get bond (port %u) MAC address: %s\n",
			       BOND_PORT, strerror(-ret));
	}

	created_pkt = rte_pktmbuf_alloc(mbuf_pool);
	if (created_pkt == NULL) {
		cmdline_printf(cl, "Failed to allocate mbuf\n");
		return;
	}

	pkt_size = sizeof(struct rte_ether_hdr) + sizeof(struct rte_arp_hdr);
	created_pkt->data_len = pkt_size;
	created_pkt->pkt_len = pkt_size;

	eth_hdr = rte_pktmbuf_mtod(created_pkt, struct rte_ether_hdr *);
	rte_ether_addr_copy(&bond_mac_addr, &eth_hdr->s_addr);
	memset(&eth_hdr->d_addr, 0xFF, RTE_ETHER_ADDR_LEN);
	eth_hdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_ARP);

	arp_hdr = (struct rte_arp_hdr *)(
		(char *)eth_hdr + sizeof(struct rte_ether_hdr));
	arp_hdr->arp_hardware = rte_cpu_to_be_16(RTE_ARP_HRD_ETHER);
	arp_hdr->arp_protocol = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
	arp_hdr->arp_hlen = RTE_ETHER_ADDR_LEN;
	arp_hdr->arp_plen = sizeof(uint32_t);
	arp_hdr->arp_opcode = rte_cpu_to_be_16(RTE_ARP_OP_REQUEST);

	rte_ether_addr_copy(&bond_mac_addr, &arp_hdr->arp_data.arp_sha);
	arp_hdr->arp_data.arp_sip = bond_ip;
	memset(&arp_hdr->arp_data.arp_tha, 0, RTE_ETHER_ADDR_LEN);
	arp_hdr->arp_data.arp_tip =
			  ((unsigned char *)&res->ip.addr.ipv4)[0]        |
			 (((unsigned char *)&res->ip.addr.ipv4)[1] << 8)  |
			 (((unsigned char *)&res->ip.addr.ipv4)[2] << 16) |
			 (((unsigned char *)&res->ip.addr.ipv4)[3] << 24);
	rte_eth_tx_burst(BOND_PORT, 0, &created_pkt, 1);

	rte_delay_ms(100);
	cmdline_printf(cl, "\n");
}

cmdline_parse_token_string_t cmd_obj_action_send =
	TOKEN_STRING_INITIALIZER(struct cmd_obj_send_result, action, "send");
cmdline_parse_token_ipaddr_t cmd_obj_ip =
	TOKEN_IPV4_INITIALIZER(struct cmd_obj_send_result, ip);

cmdline_parse_inst_t cmd_obj_send = {
	.f = cmd_obj_send_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "send client_ip",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_obj_action_send,
		(void *)&cmd_obj_ip,
		NULL,
	},
};

struct cmd_start_result {
	cmdline_fixed_string_t start;
};

static void cmd_start_parsed(__rte_unused void *parsed_result,
			       struct cmdline *cl,
			       __rte_unused void *data)
{
	int slave_core_id = rte_lcore_id();

	rte_spinlock_trylock(&global_flag_stru_p->lock);
	if (global_flag_stru_p->LcoreMainIsRunning == 0) {
		if (rte_eal_get_lcore_state(global_flag_stru_p->LcoreMainCore)
		    != WAIT) {
			rte_spinlock_unlock(&global_flag_stru_p->lock);
			return;
		}
		rte_spinlock_unlock(&global_flag_stru_p->lock);
	} else {
		cmdline_printf(cl, "lcore_main already running on core:%d\n",
				global_flag_stru_p->LcoreMainCore);
		rte_spinlock_unlock(&global_flag_stru_p->lock);
		return;
	}

	/* start lcore main on core != master_core - ARP response thread */
	slave_core_id = rte_get_next_lcore(rte_lcore_id(), 1, 0);
	if ((slave_core_id >= RTE_MAX_LCORE) || (slave_core_id == 0))
		return;

	rte_spinlock_trylock(&global_flag_stru_p->lock);
	global_flag_stru_p->LcoreMainIsRunning = 1;
	rte_spinlock_unlock(&global_flag_stru_p->lock);
	cmdline_printf(cl,
			"Starting lcore_main on core %d:%d "
			"Our IP:%d.%d.%d.%d\n",
			slave_core_id,
			rte_eal_remote_launch(lcore_main, NULL, slave_core_id),
			BOND_IP_1,
			BOND_IP_2,
			BOND_IP_3,
			BOND_IP_4
		);
}

cmdline_parse_token_string_t cmd_start_start =
	TOKEN_STRING_INITIALIZER(struct cmd_start_result, start, "start");

cmdline_parse_inst_t cmd_start = {
	.f = cmd_start_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "starts listening if not started at startup",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_start_start,
		NULL,
	},
};

struct cmd_help_result {
	cmdline_fixed_string_t help;
};

static void cmd_help_parsed(__rte_unused void *parsed_result,
			    struct cmdline *cl,
			    __rte_unused void *data)
{
	cmdline_printf(cl,
			"ALB - link bonding mode 6 example\n"
			"send IP	- sends one ARPrequest through bonding for IP.\n"
			"start		- starts listening ARPs.\n"
			"stop		- stops lcore_main.\n"
			"show		- shows some bond info: ex. active slaves etc.\n"
			"help		- prints help.\n"
			"quit		- terminate all threads and quit.\n"
		       );
}

cmdline_parse_token_string_t cmd_help_help =
	TOKEN_STRING_INITIALIZER(struct cmd_help_result, help, "help");

cmdline_parse_inst_t cmd_help = {
	.f = cmd_help_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "show help",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_help,
		NULL,
	},
};

struct cmd_stop_result {
	cmdline_fixed_string_t stop;
};

static void cmd_stop_parsed(__rte_unused void *parsed_result,
			    struct cmdline *cl,
			    __rte_unused void *data)
{
	rte_spinlock_trylock(&global_flag_stru_p->lock);
	if (global_flag_stru_p->LcoreMainIsRunning == 0)	{
		cmdline_printf(cl,
					"lcore_main not running on core:%d\n",
					global_flag_stru_p->LcoreMainCore);
		rte_spinlock_unlock(&global_flag_stru_p->lock);
		return;
	}
	global_flag_stru_p->LcoreMainIsRunning = 0;
	if (rte_eal_wait_lcore(global_flag_stru_p->LcoreMainCore) < 0)
		cmdline_printf(cl,
				"error: lcore_main can not stop on core:%d\n",
				global_flag_stru_p->LcoreMainCore);
	else
		cmdline_printf(cl,
				"lcore_main stopped on core:%d\n",
				global_flag_stru_p->LcoreMainCore);
	rte_spinlock_unlock(&global_flag_stru_p->lock);
}

cmdline_parse_token_string_t cmd_stop_stop =
	TOKEN_STRING_INITIALIZER(struct cmd_stop_result, stop, "stop");

cmdline_parse_inst_t cmd_stop = {
	.f = cmd_stop_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "this command do not handle any arguments",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_stop_stop,
		NULL,
	},
};

struct cmd_quit_result {
	cmdline_fixed_string_t quit;
};

static void cmd_quit_parsed(__rte_unused void *parsed_result,
			    struct cmdline *cl,
			    __rte_unused void *data)
{
	rte_spinlock_trylock(&global_flag_stru_p->lock);
	if (global_flag_stru_p->LcoreMainIsRunning == 0)	{
		cmdline_printf(cl,
					"lcore_main not running on core:%d\n",
					global_flag_stru_p->LcoreMainCore);
		rte_spinlock_unlock(&global_flag_stru_p->lock);
		cmdline_quit(cl);
		return;
	}
	global_flag_stru_p->LcoreMainIsRunning = 0;
	if (rte_eal_wait_lcore(global_flag_stru_p->LcoreMainCore) < 0)
		cmdline_printf(cl,
				"error: lcore_main can not stop on core:%d\n",
				global_flag_stru_p->LcoreMainCore);
	else
		cmdline_printf(cl,
				"lcore_main stopped on core:%d\n",
				global_flag_stru_p->LcoreMainCore);
	rte_spinlock_unlock(&global_flag_stru_p->lock);
	cmdline_quit(cl);
}

cmdline_parse_token_string_t cmd_quit_quit =
	TOKEN_STRING_INITIALIZER(struct cmd_quit_result, quit, "quit");

cmdline_parse_inst_t cmd_quit = {
	.f = cmd_quit_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "this command do not handle any arguments",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_quit_quit,
		NULL,
	},
};

struct cmd_show_result {
	cmdline_fixed_string_t show;
};

static void cmd_show_parsed(__rte_unused void *parsed_result,
			    struct cmdline *cl,
			    __rte_unused void *data)
{
	uint16_t slaves[16] = {0};
	uint8_t len = 16;
	struct rte_ether_addr addr;
	uint16_t i;
	int ret;

	for (i = 0; i < slaves_count; i++) {
		ret = rte_eth_macaddr_get(i, &addr);
		if (ret != 0) {
			cmdline_printf(cl,
				"Failed to get port %u MAC address: %s\n",
				i, strerror(-ret));
			continue;
		}

		PRINT_MAC(addr);
		printf("\n");
	}

	rte_spinlock_trylock(&global_flag_stru_p->lock);
	cmdline_printf(cl,
			"Active_slaves:%d "
			"packets received:Tot:%d Arp:%d IPv4:%d\n",
			rte_eth_bond_active_slaves_get(BOND_PORT, slaves, len),
			global_flag_stru_p->port_packets[0],
			global_flag_stru_p->port_packets[1],
			global_flag_stru_p->port_packets[2]);

	struct rte_eth_stats stats;
	for(i = 0; i < slaves_count; i++) {
		rte_eth_stats_get(i, &stats);
		printf("bond port %d in: %13"PRIu64", out: %13"PRIu64"\n", i, stats.ipackets, stats.opackets);
	}	
	
	rte_spinlock_unlock(&global_flag_stru_p->lock);
}

cmdline_parse_token_string_t cmd_show_show =
	TOKEN_STRING_INITIALIZER(struct cmd_show_result, show, "show");

cmdline_parse_inst_t cmd_show = {
	.f = cmd_show_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "this command do not handle any arguments",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_show_show,
		NULL,
	},
};

/****** CONTEXT (list of instruction) */

cmdline_parse_ctx_t main_ctx[] = {
	(cmdline_parse_inst_t *)&cmd_start,
	(cmdline_parse_inst_t *)&cmd_obj_send,
	(cmdline_parse_inst_t *)&cmd_stop,
	(cmdline_parse_inst_t *)&cmd_show,
	(cmdline_parse_inst_t *)&cmd_quit,
	(cmdline_parse_inst_t *)&cmd_help,
	NULL,
};

/* prompt function, called from main on MASTER lcore */
void prompt(__rte_unused void *arg1)
{
	struct cmdline *cl;

	cl = cmdline_stdin_new(main_ctx, "bond6>");
	if (cl != NULL) {
		cmdline_interact(cl);
		cmdline_stdin_exit(cl);
	}
}

void * cmd_proc(void *arg)
{
	prompt(NULL);
	return NULL;
}


