DESCRIPTION = "scull kernel module from LDD3"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = "file://Makefile \
           file://main.c \
           file://pipe.c \
           file://access.c \
           file://scull.h \
           file://access_ok_version.h \
           file://proc_ops_version.h"
S = "${WORKDIR}"



EXTRA_OEMAKE += "KERNELDIR=${STAGING_KERNEL_DIR} -C ${STAGING_KERNEL_DIR} M=${S}"

