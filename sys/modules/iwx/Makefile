# $FreeBSD$

.PATH:  ${SRCTOP}/sys/dev/iwx

KMOD=	if_iwx
SRCS=	if_iwx.c
SRCS+=	device_if.h bus_if.h pci_if.h opt_wlan.h opt_iwm.h

CFLAGS+= -DIWX_DEBUG

.include <bsd.kmod.mk>
