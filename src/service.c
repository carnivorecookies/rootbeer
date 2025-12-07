// for fchown(2)
#include <security/_pam_types.h>
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <err.h>
#include <grp.h>
#include <pwd.h>
#include <security/pam_appl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "service.h"
#include "socket.h"

/*
 * wheel_gid - get the gid of the wheel group
 * Returns the gid of the wheel group, or returns -1 on error.
 * The result is cached, so you can call the function multiple times and
 * expect the same result.
 */
static inline gid_t
wheel_gid(void)
{
	gid_t gid;
	if (gid_from_group("wheel", &gid) < 0) {
		warn("no wheel group found");
		return -1;
	}
	return gid;
}

int
service_start(void)
{
	struct sockaddr_un addr = { AF_UNIX, SERVICE_SOCK_PATH };
	int sock;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		return -1;

	if (wheel_gid() < 0)
		return -1;

	// root:wheel
	if (fchown(sock, 0, wheel_gid()) < 0)
		return -1;

	// -rw-rw----
	if (fchmod(sock, 0o660) < 0)
		return -1;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return -1;

	if (listen(sock, 1) < 0)
		return -1;

	return sock;
}

bool
service_client_authorized(int sock)
{
	gid_t gid, *groups, wheel;
	uid_t uid;
	int i, ngroups;
	const char *user;

	if (getpeereid(sock, &uid, &gid) < 0)
		return false;

	if (gid_from_group("wheel", &wheel) < 0)
		return false;

	user = user_from_uid(uid, 1);
	if (user == NULL)
		return false;

	getgrouplist(user, gid, NULL, &ngroups);

	groups = malloc(ngroups * sizeof(*groups));
	if (groups == NULL)
		return false;

	getgrouplist(user, gid, groups, &ngroups);

	for (i = 0; i < ngroups; i++) {
		if (groups[i] == wheel) {
			free(groups);
			return true;
		}
	}

	free(groups);
	return false;
}

static int
auth_user_conv(int num_msg, const struct pam_message **msg,
    struct pam_response **resp, void *sockptr)
{
	size_t len;
	int sock = *(int *)sockptr;
	char *passwd;

	// resp_retcode must be 0, and the unused responses should be NULL
	*resp = calloc(num_msg, sizeof(**resp));
	if (*resp == NULL)
		return PAM_BUF_ERR;

	passwd = recv_all(sock, &len);
	if (passwd == NULL)
		return PAM_BUF_ERR;

	(*resp)[0].resp = passwd;

	return PAM_SUCCESS;
}

static bool
authenticate(int sock)
{
	struct pam_conv conv = { .conv = &auth_user_conv };
	pam_handle_t *pamh;
	struct passwd *pwd = NULL, *result;
	long buflen;
	uid_t uid;
	char *buf = NULL;
	bool authed = false;

	pwd = malloc(sizeof(*pwd));
	if (pwd == NULL)
		return false;

	buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (buflen < 0)
		buflen = 16384;

	buf = malloc(buflen);
	if (buf == NULL)
		goto done;

	if (getpeereid(sock, &uid, NULL) < 0)
		goto done;

	getpwuid_r(uid, pwd, buf, buflen, &result);
	if (result == NULL)
		goto done;

	pam_start("rootbeer", pwd->pw_name, &conv, &pamh);

	if (pam_authenticate(pamh, PAM_SILENT) != PAM_SUCCESS)
		goto done;

	authed = true;
done:
	free(buf);
	free(pwd);
	pam_end(pamh, PAM_SUCCESS);
	return authed;
}

int
service_handle_client(int sock)
{
	if (authenticate(sock) == false) {
		warnx("authentication failure");
		return -1;
	}

	return 0;
}