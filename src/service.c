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

/**
 * Gets the gid of the wheel group.
 *
 * @retval The gid of the wheel group.
 * @retval `-1` on error
 *
 * @note The result is cached, so you can call the function multiple times and
 * expect the same result.
 */
static __inline gid_t
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
service_init(void)
{
	if (wheel_gid() < 0) {
		warnx("wheel account does not exist");
		return -1;
	}

	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		return -1;

	struct sockaddr_un addr = { AF_UNIX, SERVICE_SOCK_PATH };
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return -1;

	// -rw-rw----
	if (chmod(SERVICE_SOCK_PATH, 0660) < 0)
		return -1;

	// root:wheel
	if (chown(SERVICE_SOCK_PATH, 0, wheel_gid()) < 0)
		return -1;

	if (listen(sock, 1) < 0)
		return -1;

	return sock;
}

bool
service_client_authorized(int sock)
{
	gid_t gid;
	uid_t uid;
	if (getpeereid(sock, &uid, &gid) < 0)
		return false;

	gid_t wheel;
	if (gid_from_group("wheel", &wheel) < 0)
		return false;

	const char *user = user_from_uid(uid, 1);
	if (user == NULL)
		return false;

	// get number of groups
	int ngroups;
	getgrouplist(user, gid, NULL, &ngroups);

	gid_t *groups = malloc(ngroups * sizeof(*groups));
	if (groups == NULL)
		return false;

	getgrouplist(user, gid, groups, &ngroups);

	for (int i = 0; i < ngroups; i++) {
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
	int sock = *(int *)sockptr;

	// resp_retcode must be 0, and the unused responses should be NULL
	*resp = calloc(num_msg, sizeof(**resp));
	if (*resp == NULL)
		return PAM_BUF_ERR;

	char *passwd = recv_all(sock);
	if (passwd == NULL)
		return PAM_BUF_ERR;

	(*resp)[0].resp = passwd;

	return PAM_SUCCESS;
}

static bool
authenticate(int sock)
{
	pam_handle_t *pamh = NULL;
	bool authed = false;

	uid_t uid;
	gid_t gid;
	if (getpeereid(sock, &uid, &gid) < 0)
		return -1;

	struct passwd *pwd = malloc(sizeof(*pwd));
	if (pwd == NULL)
		return -1;

	long buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (buflen < 0)
		buflen = 16384;

	char *buf = malloc(buflen);
	if (buf == NULL)
		goto done;

	struct passwd *result;
	getpwuid_r(uid, pwd, buf, buflen, &result);
	if (result == NULL)
		goto done;

	struct pam_conv conv = { .conv = &auth_user_conv,
		.appdata_ptr = &sock };
	if (pam_start("rootbeer", pwd->pw_name, &conv, &pamh) != PAM_SUCCESS) {
		pamh = NULL;
		goto done;
	}

	if (pam_authenticate(pamh, PAM_SILENT) != PAM_SUCCESS)
		goto done;

	authed = true;
done:
	free(buf);
	free(pwd);
	if (pamh != NULL)
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