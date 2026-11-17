// SPDX-License-Identifier: MIT
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>

#include "state.h"

/* Fill @out with the state directory path. 0 on success, -1 on error. */
static int state_dir(char *out, size_t len)
{
	const char *base = getenv("XDG_STATE_HOME");
	const char *home;
	int n;

	if (base && base[0] == '/') {
		n = snprintf(out, len, "%s/kernelwatch", base);
	} else {
		home = getenv("HOME");
		if (!home || home[0] != '/')
			return -1;
		n = snprintf(out, len, "%s/.local/state/kernelwatch", home);
	}

	if (n < 0 || (size_t)n >= len)
		return -1;
	return 0;
}

/* Like `mkdir -p`. Modifies @path temporarily but restores it. */
static int mkdir_p(char *path, mode_t mode)
{
	char *p;

	for (p = path + 1; *p; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		if (mkdir(path, mode) < 0 && errno != EEXIST) {
			*p = '/';
			return -1;
		}
		*p = '/';
	}

	if (mkdir(path, mode) < 0 && errno != EEXIST)
		return -1;
	return 0;
}

int state_write(const char *ver)
{
	char dir[PATH_MAX];
	char tmp[PATH_MAX + 16], dst[PATH_MAX + 16];
	FILE *f;

	if (state_dir(dir, sizeof(dir)) < 0)
		return -1;
	if (mkdir_p(dir, 0700) < 0)
		return -1;

	if (snprintf(tmp, sizeof(tmp), "%s/last.tmp", dir) >=
		    (int)sizeof(tmp) ||
	    snprintf(dst, sizeof(dst), "%s/last", dir) >= (int)sizeof(dst))
		return -1;

	f = fopen(tmp, "w");
	if (!f)
		return -1;

	if (fprintf(f, "%s\n", ver) < 0) {
		fclose(f);
		goto err;
	}
	if (fclose(f) != 0)
		goto err;

	if (rename(tmp, dst) < 0)
		goto err;
	return 0;

err:
	unlink(tmp);
	return -1;
}

/*
 * Read the last known version into @out.
 * Returns 0 on success, -1 if the file is missing, unreadable, or empty
 * (the caller treats all of these as "no cache yet").
 */
int state_read(char *out, size_t len)
{
	char dir[PATH_MAX];
	char path[PATH_MAX + 16];
	FILE *f;

	if (len == 0)
		return -1;
	if (state_dir(dir, sizeof(dir)) < 0)
		return -1;
	if (snprintf(path, sizeof(path), "%s/last", dir) >= (int)sizeof(path))
		return -1;

	f = fopen(path, "r");
	if (!f)
		return -1;

	if (!fgets(out, (int)len, f)) {
		fclose(f);
		return -1;
	}
	fclose(f);

	out[strcspn(out, "\r\n")] = '\0';

	if (out[0] == '\0')
		return -1;
	return 0;
}
