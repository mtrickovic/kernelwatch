// SPDX-License-Identifier: MIT
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "notify.h"

void notify_new_version(const char *latest, const char *running)
{
	char body[128];
	pid_t pid;
	int fd;

	snprintf(body, sizeof(body), "Stable %s is out (running %s)", latest,
		 running);

	pid = fork();
	if (pid != 0)
		return;

	/* child */
	fd = open("/dev/null", O_RDWR);
	if (fd >= 0) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			close(fd);
	}
	signal(SIGPIPE, SIG_DFL);

	execlp("notify-send", "notify-send", "-a", "kernelwatch",
	       "Linux kernel update", body, (char *)NULL);
	_exit(127);
}
