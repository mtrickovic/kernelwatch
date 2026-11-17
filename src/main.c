// SPDX-License-Identifier: MIT
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "net.h"
#include "parser.h"
#include "version.h"
#include "state.h"
#include "notify.h"

#define ICON "\uf17c" /* Nerd Font tux */
#define CLASS_UPTODATE "uptodate"
#define CLASS_OUTDATED "outdated"
#define CLASS_ERROR "error"
#define CLASS_NEW "new"
#define CLASS_STALE "stale"

#define MSG_TEXT "{\"text\":\"" ICON " ?\","
#define MSG_TOOLTIP "\"tooltip\":\"kernelwatch: timed out\","
#define MSG_CLASS "\"class\":[\"" CLASS_ERROR "\"]}\n"

/* Write @s as a JSON string literal, including the quotes. */
static void put_json_str(FILE *out, const char *s)
{
	fputc('"', out);
	for (; *s; s++) {
		unsigned char c = (unsigned char)*s;

		switch (c) {
		case '"':
			fputs("\\\"", out);
			break;
		case '\\':
			fputs("\\\\", out);
			break;
		case '\n':
			fputs("\\n", out);
			break;
		case '\r':
			fputs("\\r", out);
			break;
		case '\t':
			fputs("\\t", out);
			break;
		default:
			if (c < 0x20)
				fprintf(out, "\\u%04x", c);
			else
				fputc(c, out);
		}
	}
	fputc('"', out);
}

/* One line of waybar JSON. Waybar reads line by line, so flush. */
static void print_waybar(const char *text, const char *tooltip,
			 const char *const *classes, size_t n)
{
	size_t i;

	fputs("{\"text\":", stdout);
	put_json_str(stdout, text);
	fputs(",\"tooltip\":", stdout);
	put_json_str(stdout, tooltip);
	fputs(",\"class\":[", stdout);
	for (i = 0; i < n; i++) {
		if (i)
			fputc(',', stdout);
		put_json_str(stdout, classes[i]);
	}
	fputs("]}\n", stdout);
	fflush(stdout);
}

static void on_alarm(int sig)
{
	static const char msg[] = MSG_TEXT MSG_TOOLTIP MSG_CLASS;

	(void)sig;
	(void)!write(STDOUT_FILENO, msg, sizeof(msg) - 1);
	_exit(0);
}

int main(void)
{
	static const char *const err_class[] = { CLASS_ERROR };
	char buf[NET_RESP_SIZE];
	char latest[32], cached[32], running[32];
	char text[64], tip[160];
	const char *classes[2];
	size_t n = 0;
	int have_cache, fetched, is_new = 0;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, on_alarm);
	alarm(20);

	have_cache = state_read(cached, sizeof(cached)) == 0;

	fetched = fetch_banner(buf, sizeof(buf)) == 0 &&
		  parse_stable_version(buf, latest, sizeof(latest)) == 0;

	if (fetched) {
		is_new = have_cache && strcmp(cached, latest) != 0;
		if (!have_cache || is_new)
			state_write(latest);
	} else if (have_cache) {
		snprintf(latest, sizeof(latest), "%s", cached);
	} else {
		fprintf(stderr, "kernelwatch: fetch failed, no cache\n");
		print_waybar(ICON " ?", "kernelwatch: fetch failed", err_class,
			     1);
		return EXIT_SUCCESS;
	}

	if (!fetched)
		fprintf(stderr, "kernelwatch: fetch failed, using cache\n");

	if (get_running_version(running, sizeof(running)) < 0)
		snprintf(running, sizeof(running), "unknown");

	snprintf(text, sizeof(text), ICON " %s", latest);
	snprintf(tip, sizeof(tip), "Running: %s\nLatest stable: %s%s", running,
		 latest, fetched ? "" : "\n(cached, offline)");

	if (!fetched)
		classes[n++] = CLASS_STALE;
	else
		classes[n++] = strcmp(running, latest) == 0 ? CLASS_UPTODATE :
							      CLASS_OUTDATED;

	if (is_new)
		classes[n++] = CLASS_NEW;

	print_waybar(text, tip, classes, n);
	if (is_new)
		notify_new_version(latest, running);
	return EXIT_SUCCESS;
}
