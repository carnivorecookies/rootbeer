#include <stddef.h>

/**
 * `recv()` a list of strings from a socket.
 * @param fd File descriptor of the socket.
 *
 * @retval a `NULL`-terminated, allocated array of `NUL`-terminated allocated
 * strings, which all must be freed by the caller.
 * @retval `NULL` on error.
 */
char **recv_array(int fd);

/**
 * `send()` a list of strings to a socket.
 * @param fd File descriptor of the socket.
 * @param strings A `NULL`-terminated array of `NUL`-terminated strings.
 *
 * @retval `0` on success.
 * @retval -1 on error.
 */
int send_array(int fd, char **strings);

/**
 * `recv()` a full message from a socket (prefixed with its length)/
 * @param fd socket file descriptor
 *
 * @retval A `NUL`-terminated string on success, which must be freed by the
 * caller.
 * @retval `NULL` on error.
 */
void *recv_all(int fd);

/**
 * `send()` a full message to a socket (prefixed with its length).
 * @param fd File descriptor of the socket.
 * @param buf The text to send.
 *
 * @retval `0` on success.
 * @retval `-1` on error.
 */
int send_all(int fd, char *buf);

/**
 * Closes and shuts down a socket.
 * @param fd Socket file descriptor.
 *
 * @note If close or shutdown of the socket fails, shutdown_close calls
 * `warn()`.
 */
void shutdown_close(int fd);