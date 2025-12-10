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
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "failed to create socket");

	struct sockaddr_un service = { AF_UNIX, SERVICE_SOCK_PATH };
	if (connect(sock, (struct sockaddr *)&service, sizeof(service)) < 0)
		err(1, "failed to connect to service");

	char pass[PAM_MAX_RESP_SIZE];
	if (readpassphrase("enter password: ", pass, sizeof(pass), 0) == NULL)
		err(1, "failed to read password");

	if (send_all(sock, pass) < 0)
		err(1, "failed to send to service");
}

int
main(int argc, char **argv)
{
	(void)argv;
	if (argc > 1) {
		run_client();
		return 0;
	}

	int sock = service_init();
	if (sock < 0)
		err(1, "failed to init daemon");

	int client = accept(sock, NULL, NULL);
	if (client < 0)
		err(1, "connection failed");

	if (!service_client_authorized(client)) {
		shutdown_close(client);
		warnx("client not authorized");
	}

	service_handle_client(client);
}