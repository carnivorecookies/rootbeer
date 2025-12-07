#include <sys/cdefs.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <readpassphrase.h>
#include <security/pam_appl.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "service.h"
#include "socket.h"

static void
run_client(void)
{
	struct sockaddr_un service = { AF_UNIX, SERVICE_SOCK_PATH };
	int sock;
	char pass[PAM_MAX_RESP_SIZE];

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "failed to create socket");

	if (connect(sock, (struct sockaddr *)&service, sizeof(service)) < 0)
		err(1, "failed to connect to service");

	if (readpassphrase("enter password: ", pass, sizeof(pass), 0) == NULL)
		err(1, "failed to read password");

	if (send_all(sock, pass) < 0)
		err(1, "failed to send to service");
}

int
main(int argc, char **argv)
{
	int sock, client;

	(void)argv;
	if (argc > 1) {
		run_client();
		return 0;
	}

	sock = service_init();
	if (sock < 0)
		err(1, "failed to init daemon");

	client = accept(sock, NULL, NULL);
	if (client < 0)
		err(1, "connection failed");

	if (!service_client_authorized(client)) {
		shutdown_close(client);
		warnx("client not authorized");
	}

	service_handle_client(client);
}