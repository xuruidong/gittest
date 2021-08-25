/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#ifndef _MAIN_H_
#define _MAIN_H_

struct global_flag_stru_t {
	int LcoreMainIsRunning;
	int LcoreMainCore;
	uint32_t port_packets[4];
	rte_spinlock_t lock;
};

int lcore_main(__rte_unused void *arg1);

int main(int argc, char *argv[]);

void prompt(__rte_unused void *arg1);
void * cmd_proc(void *arg);



#endif /* ifndef _MAIN_H_ */
