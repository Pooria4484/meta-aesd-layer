DESCRIPTION = "Misc modules from ldd3"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = "file://misc-modules/"

S = "${WORKDIR}/misc-modules"

EXTRA_OEMAKE += "KERNELDIR=${STAGING_KERNEL_DIR} -C ${STAGING_KERNEL_DIR} M=${S}"

PACKAGES =+ "kernel-module-hello kernel-module-faulty"

FILES:kernel-module-hello = "/lib/modules/${KERNEL_VERSION}/extra/hello.ko"
FILES:kernel-module-faulty = "/lib/modules/${KERNEL_VERSION}/extra/faulty.ko"

do_install:append() {
    install -d ${D}/lib/modules/${KERNEL_VERSION}/extra
    install -m 0644 ${S}/hello.ko ${D}/lib/modules/${KERNEL_VERSION}/extra/
    install -m 0644 ${S}/faulty.ko ${D}/lib/modules/${KERNEL_VERSION}/extra/
    depmod -a -b ${D} ${KERNEL_VERSION}
}

FILES:${PN} += "/lib/modules"
