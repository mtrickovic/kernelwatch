// SPDX-License-Identifier: MIT
#include <string.h>
#include <sys/utsname.h>

#include "version.h"

/*
 * Returns 0 and fills @out with e.g. "7.3.3", or -1 if the line
 * is missing or the version doesn't fit in @out.
 */
int get_running_version(char *out, size_t len)
{
	struct utsname u;
	size_t n;

	if (uname(&u) < 0)
		return -1;

	n = strspn(u.release, "0123456789.");
	if (n == 0 || n >= len)
		return -1;

	memcpy(out, u.release, n);
	out[n] = '\0';
	return 0;
}
