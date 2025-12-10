#include <sys/types.h>
#include <sys/socket.h>

#include <assert.h>
#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"

void *
recv_all(int fd)
{
	// The first bits are the length of the message.
	size_t len;
	if (recv(fd, &len, sizeof(len), MSG_WAITALL) < (ssize_t)sizeof(len))
		return NULL;

	// Allocate one more byte in case the sent string isn't NUL-terminated.
	char *buf = malloc(len + 1);
	if (buf == NULL)
		return NULL;

	// If the amount of bytes read is -1 or less than the length, a signal
	// or interrupt has occurred.
	ssize_t read = recv(fd, buf, len, MSG_WAITALL);
	if (read < 0 || (size_t)read < len) {
		free(buf);
		return NULL;
	}

	// this is guaranteed, but some LSPs dislike if this assertion does not
	// exist.
	assert(read > 0);
	if (buf[read - 1] != '\0') {
		buf[read] = '\0';
	}

	return buf;
}

/**
 * Similar to send_all, but without sending a length prefix.
 * @param fd Socket file descriptor.
 * @param buf The text to send.
 * @param len The length of the text to send.
 *
 * @retval `0` on success.
 * @retval `-1` on error.
 */
static int
send_all_raw(int fd, char *buf, size_t len)
{
	size_t totalsent = 0;
	while (totalsent < len) {
		ssize_t sent = send(fd, buf + totalsent, len - totalsent, 0);
		if (sent < 0)
			return -1;
		totalsent += sent;
	}

	return 0;
}

int
send_all(int fd, char *buf)
{
	size_t len = strlen(buf);
	if (len < 0)
		return 0;

	// Prepend the bits of the message length
	if (send_all_raw(fd, (char *)&len, sizeof(len)) < 0)
		return -1;

	if (send_all_raw(fd, buf, len) < 0)
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