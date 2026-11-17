// SPDX-License-Identifier: MIT
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "net.h"

/*
 * Reads the whole response into @buf (NUL-terminated).
 * Returns bytes read, or -1 on error or if it doesn't fit.
 */
static ssize_t read_response(SSL *ssl, char *buf, size_t len)
{
	size_t used = 0;
	int n, err;

	for (;;) {
		if (used + 1 >= len) {
			fprintf(stderr, "reponse too large\n");
			return -1;
		}

		n = SSL_read(ssl, buf + used, (int)(len - 1 - used));
		if (n > 0) {
			used += (size_t)n;
			continue;
		}

		err = SSL_get_error(ssl, n);
		if (err == SSL_ERROR_ZERO_RETURN)
			break;
		ERR_print_errors_fp(stderr);
		return -1;
	}

	buf[used] = '\0';
	return (ssize_t)used;
}

/* In place: verify 200 and move the body to the start of buf. */
static int strip_http_headers(char *buf, size_t used)
{
	char *body;

	if (strncmp(buf, "HTTP/1.", 7) != 0 || strncmp(buf + 8, " 200", 4) != 0)
		return -1;

	body = strstr(buf, "\r\n\r\n");
	if (!body)
		return -1;
	body += 4;

	memmove(buf, body, used - (size_t)(body - buf) + 1);
	return 0;
}

int fetch_banner(char *buf, size_t size)
{
	const char *hostname = "www.kernel.org";
	const char *path = "/finger_banner";
	int s, sfd = -1, len;
	int ret = -1;
	ssize_t used = 0;
	struct addrinfo hints;
	struct addrinfo *result = NULL, *rp;
	SSL *ssl = NULL;
	SSL_CTX *ctx = NULL;

	if (size < NET_REQ_SIZE)
		return -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	s = getaddrinfo(hostname, "443", &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		close(sfd);
		sfd = -1;
	}
	freeaddrinfo(result);

	if (sfd == -1) {
		fprintf(stderr, "connect: %s\n", strerror(errno));
		return -1;
	}

	ctx = SSL_CTX_new(TLS_client_method());
	if (!ctx)
		goto err_ssl;
	if (!SSL_CTX_set_default_verify_paths(ctx))
		goto err_ssl;
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	ssl = SSL_new(ctx);
	if (!ssl)
		goto err_ssl;
	if (!SSL_set_fd(ssl, sfd))
		goto err_ssl;
	if (!SSL_set_tlsext_host_name(ssl, hostname))
		goto err_ssl;
	if (!SSL_set1_dnsname(ssl, hostname))
		goto err_ssl;
	if (SSL_connect(ssl) != 1)
		goto err_ssl;

	len = snprintf(buf, NET_REQ_SIZE,
		       "GET %s HTTP/1.0\r\n"
		       "Host: %s\r\n"
		       "User-Agent: kernelwatch/0.1\r\n"
		       "Connection: close\r\n\r\n",
		       path, hostname);
	if (len < 0 || len >= NET_REQ_SIZE) {
		fprintf(stderr, "request too long\n");
		goto out;
	}

	if (SSL_write(ssl, buf, len) != len)
		goto err_ssl;

	used = read_response(ssl, buf, size);
	if (used < 0)
		goto out;

	if (strip_http_headers(buf, (size_t)used) < 0)
		goto out;

	ret = 0;
	goto out;

err_ssl:
	ERR_print_errors_fp(stderr);
out:
	if (ssl)
		SSL_free(ssl);
	if (ctx)
		SSL_CTX_free(ctx);
	close(sfd);
	return ret;
}
