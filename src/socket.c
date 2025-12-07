#include <sys/types.h>
#include <sys/socket.h>

#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "socket.h"

void *
recv_all(int fd, size_t *len)
{
	ssize_t read;
	char *buf;

	// The first bits are the length of the message.
	if (recv(fd, len, sizeof(*len), MSG_WAITALL) < (ssize_t)sizeof(*len))
		return NULL;

	// The rest of the message is the actual message, with len storing the
	// full message length.
	buf = malloc(*len);
	if (buf == NULL)
		return NULL;

	// If the amount of bytes read is -1 or less than the length, a signal
	// or interrupt has occurred.
	if ((read = recv(fd, buf, *len, MSG_WAITALL)) < 0 ||
	    (size_t)read < *len) {
		free(buf);
		return NULL;
	}

	return buf;
}

/*
 * send_all_raw - same as send_all, but without sending a length prefix.
 * @fd - socket file descriptor
 * @buf - the text to send
 * @len - the length of the text to send
 *
 * Returns 0 on success and -1 on error.
 */
static int
send_all_raw(int fd, void *buf, size_t len)
{
	ssize_t sent;
	size_t totalsent = 0;
	char *cbuf = (char *)buf;

	while (totalsent < len) {
		sent = send(fd, cbuf + totalsent, len - totalsent, 0);
		if (sent < 0)
			return -1;
		totalsent += sent;
	}

	return 0;
}

int
send_all(int fd, void *buf, size_t n)
{
	// Prepend the bits of the message length
	if (send_all_raw(fd, &n, sizeof(n)) < 0)
		return -1;

	if (send_all_raw(fd, buf, n) < 0)
		return -1;

	return 0;
}

void
shutdown_close(int fd)
{
	if (shutdown(fd, SHUT_RDWR) < 0)
		warn("failed to shut down socket");
	if (close(fd) < 0)
		warn("failed to close socket");
}