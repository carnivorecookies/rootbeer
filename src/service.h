#ifndef SERVICE_H
#define SERVICE_H

#define SERVICE_SOCK_PATH "/tmp/rootbeer.sock"

#include <stdbool.h>
struct pam_handle;

/*
 * service_start - starts the rootbeer service and puts it into listening mode
 *
 * Returns the file descriptor of the socket, or -1 on error.
 * pam_start(3) must be called before calling service_start.
 */
int service_start(void);

/*
 * service_client_authorized - checks whether the connected user is authorized
 * @sock - file descriptor of the socket
 *
 * This function is not thread safe, and must be called before
 * service_handle_client.
 *
 * Returns true if the user is authorized, or returns
 * false if the user is not authorized or an error has occurred.
 */
bool service_client_authorized(int sock);

/*
 * service_client_authorized - checks whether the connected user is authorized
 * @sock - file descriptor of the socket
 *
 * This function is not thread safe, and must be called before
 * service_handle_client.
 *
 * Returns true if the user is authorized, and returns
 * false if the user is not authorized or if an error has occurred.
 */
int service_handle_client(int fd);

#endif // SERVICE_H