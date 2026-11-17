/* SPDX-License-Identifier: MIT */
#ifndef KERNELWATCH_NET_H
#define KERNELWATCH_NET_H

#include <stddef.h>

#define NET_RESP_PORT 443
#define NET_RESP_SIZE 16384
#define NET_REQ_SIZE 1024

int fetch_banner(char *buf, size_t size);

#endif /* KERNELWATCH_NET_H */
