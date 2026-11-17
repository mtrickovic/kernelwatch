/* SPDX-License-Identifier: MIT */
#ifndef KERNELWATCH_STATE_H
#define KERNELWATCH_STATE_H

#include <stddef.h>

int state_read(char *out, size_t len);

int state_write(const char *ver);

#endif /* KERNELWATCH_STATE_H */
