#ifndef SERVICE_H
#define SERVICE_H

#define SERVICE_SOCK_PATH "/tmp/rootbeer.sock"

#include <stdbool.h>
struct pam_handle;

/**
 * Starts the rootbeer service and puts it into listening mode.
 *
 * @retval The file descriptor of the socket.
 * @retval `-1` on error.
 */
int service_init(void);

/**
 * Checks whether the connected user at `sock` is authorized.
 *
 * @param sock The file descriptor of the socket.
 *
 * @note This function is not thread safe, and must be called before `service_handle_client`.
 *
 * @retval `true` if the user is authorized.
 * @retval `false` if the user is not authorized.
 * @retval `false` if an error has occurred.
 */
bool service_client_authorized(int sock);

/**
 * Handles the client at `sock`, running commands and performing authentication.
 *
 * @param sock The file descriptor of the socket.
 *
 * @retval `true` if the user is authorized.
 * @retval `false` if the user is not authorized.
 * @retval `false` if an error has occurred.
 */
int service_handle_client(int sock);

#endif // SERVICE_H