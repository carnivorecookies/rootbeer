#include <stddef.h>

/*
 * recv_all - recv a full message through a socket (prefixed with its length)
 * @fd - socket file descriptor
 * @plen - the number of bytes long the returned data is.
 *
 * Returns allocated data on success, which must be freed by the caller.
 * Returns NULL on error.
 */
void *recv_all(int fd, size_t *plen);

/*
 * send_all - send a full message through a socket (prefixed with its length)
 * @fd - socket file descriptor
 * @buf - the text to send
 * @n - the length of the text to send
 *
 * Returns 0 on success and -1 on error.
 */
int send_all(int fd, void *buf, size_t n);

/*
 * shutdown_close - closes and shuts down a socket
 * @fd - socket file descriptor
 *
 * If close or shutdown of the socket fails, shutdown_close calls warn(3).
 */
void shutdown_close(int fd);