/*
 * Copyright (C) 2018 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */
/* Connection handle for a TCP Client session */

#include "tcp_perf_client.h"
/** The report function of a TCP client session */

static struct tcp_pcb *c_pcb;
u8 connectFlags;
static void tcp_client_close(struct tcp_pcb *pcb)
{
	err_t err;

	if (pcb != NULL) {
		tcp_sent(pcb, NULL);
		tcp_err(pcb, NULL);
		err = tcp_close(pcb);
		if (err != ERR_OK) {
			/* Free memory with abort */
			tcp_abort(pcb);
		}
	}
}


void setCallbackFunction(tcp_recv_fn recv)
{
	tcp_recv(c_pcb, recv);
}
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	if (err != ERR_OK)
	{
		tcp_client_close(tpcb);
		xil_printf("Connection error\n\r");
		return err;
	}
	/* store state */
	c_pcb = tpcb;
	/* set callback values & functions */
	connectFlags = 1;
	/* initiate data transfer */
	return ERR_OK;
}

int start_application(void)
{
	err_t err;
	struct tcp_pcb *pcb;
	ip_addr_t remote_addr;

	err = inet_aton(TCP_SERVER_IP_ADDRESS, &remote_addr);

	if (!err)
	{
		xil_printf("Invalid Server IP address: %d\r\n", err);
		return 0;
	}

	/* Create Client PCB */
	pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb)
	{
		xil_printf("Error in PCB creation. out of memory\r\n");
		return 0;
	}

	err = tcp_connect(pcb, &remote_addr, TCP_CONN_PORT,
					  tcp_client_connected);
	if (err)
	{
		xil_printf("Error on tcp_connect: %d\r\n", err);
		tcp_client_close(pcb);
		return 0;
	}
	while(connectFlags != 1);
	connectFlags = 0;
	return 1;
}
