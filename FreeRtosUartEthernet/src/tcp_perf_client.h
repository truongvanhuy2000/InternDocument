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

#ifndef __FREERTOS_TCP_PERF_CLIENT_H_
#define __FREERTOS_TCP_PERF_CLIENT_H_

// other things :))
#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "lwip/inet.h"
#include "xil_printf.h"
#include "ApplicationData.h"

#define TCP_CLIENT_THREAD_STACKSIZE 2048

/* Client port to connect */
#define TCP_CONN_PORT 5001

/* time in seconds to transmit packets */
#define TCP_TIME_INTERVAL 300

/* Server to connect with */
#define TCP_SERVER_IP_ADDRESS "192.168.1.100"

static QueueHandle_t ethernetRxQueue;

int start_application(void);
void setCallbackFunction(tcp_recv_fn recv);
#endif
