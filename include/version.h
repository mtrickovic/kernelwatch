/* SPDX-License-Identifier: MIT */
#ifndef KERNELWATCH_VERSION_H
#define KERNELWATCH_VERSION_H

#include <stddef.h>

/*
 * Returns 0 and fills @out with e.g. "7.3.3", or -1 if the line
 * is missing or the version doesn't fit in @out.
 */
int get_running_version(char *out, size_t len);

#endif /* KERNELWATCH_VERSION_H */
