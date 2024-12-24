# $FreeBSD$

# Common rules for building iwx firmware.

#EXT?=	uu
#_NAME?=		iwx-${NAME}-${FWVERSION}.${EXT}
_NAME?=		iwx-${NAME}-${FWVERSION}

IMG=		${_NAME}
KMOD=		${_NAME}

CLEANFILES+=	${IMG}

FIRMWS=		${IMG}:${IMG}:${FWVERSION}

# License ack is not needed for iwlwifi (same as iwn/iwm).
#FIRMWARE_LICENSE=

${IMG}: 
	uudecode -p ${SRCTOP}/sys/contrib/dev/iwx/${IMG}.uu > ${.TARGET}
