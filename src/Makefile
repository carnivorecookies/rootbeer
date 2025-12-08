.include <bsd.own.mk>

PROG=		beer

SRCS=	rootbeer.c service.c socket.c
MAN=

WARNS?= 	2
CFLAGS+=	-std=c17 ${DEP_C}
LDADD+=		-lpam

.if ${OS} == Linux
CFLAGS+=	-isystem ${:!echo /usr/include/*/bsd!} -DLIBBSD_OVERLAY
LDADD+=		-lbsd-overlay
.endif

.include <bsd.prog.mk>