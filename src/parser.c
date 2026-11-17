// SPDX-License-Identifier: MIT
#include <string.h>
#include <sys/utsname.h>

#include "parser.h"

int parse_stable_version(const char *buf, char *out, size_t out_len)
{
	static const char key[] =
		"The latest stable version of the Linux kernel is:";
	const char *p;
	size_t n;

	p = strstr(buf, key);
	if (!p)
		return -1;

	p += sizeof(key) - 1; /* skip the key itself */
	p += strspn(p, " \t"); /* skip the padding */
	n = strcspn(p, " \t\r\n"); /* version ends at whitespace */

	if (n == 0 || n >= out_len)
		return -1;

	memcpy(out, p, n);
	out[n] = '\0';
	return 0;
}
