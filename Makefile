PROG=	beer
SRCS!=	ls src/*.c
MAN=

PKG_CONFIG?=	pkgconf

DEP_C!=		${PKG_CONFIG} --cflags libbsd-overlay pam
DEP_LD!=	${PKG_CONFIG} --libs   libbsd-overlay pam

.if empty(DEP_LD)
.error missing dependencies
.endif

WARNS ?= 2

CFLAGS+=	-std=c17 ${DEP_C}
LDADD+=		${DEP_LD}

.include <bsd.prog.mk>