.include <bsd.own.mk>

PROG=	../beer

SRCS=	main.c service.c socket.c
MAN=

WARNS?= 2

CFLAGS+=	-std=c17
LDADD+=		-lpam

.if ${WARNS} > 1
CFLAGS+= -Wpedantic
.endif

.if ${OS} == Linux
CFLAGS+=	-isystem ${:!echo /usr/include/*/bsd!} -DLIBBSD_OVERLAY
LDADD+=		-lbsd
.endif

.include <bsd.prog.mk>