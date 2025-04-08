DESCRIPTION = "AESD character device driver"
LICENSE = "GPLv2"

inherit module

SRC_URI = "file://."

S = "${WORKDIR}"

EXTRA_OEMAKE += "KERNELDIR=${STAGING_KERNEL_DIR} -C ${STAGING_KERNEL_DIR} M=${S}"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0644 aesdchar.ko ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
}

do_populate_lic[noexec] = "1"
