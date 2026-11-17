/* SPDX-License-Identifier: MIT */
#ifndef KERNELWATCH_PARSER_H
#define KERNELWATCH_PARSER_H

#include <stddef.h>

int parse_stable_version(const char *buf, char *out, size_t out_len);

#endif /* KERNELWATCH_PARSER_H */
