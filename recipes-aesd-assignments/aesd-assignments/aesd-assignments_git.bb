# See https://git.yoctoproject.org/poky/tree/meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Set your assignments repository URL using SSH protocol
# Configure SSH keys for passwordless access as shown in lecture notes
SRC_URI = "git://git@github.com/cu-ecen-aeld/assignment-6-Pooria4484.git;protocol=ssh;branch=main"

PV = "1.0+git${SRCPV}"
# Replace with your specific commit hash from the assignment repo
SRCREV = "fa7023c33d5c0ea84653270eee0af22dc28c3689"

# Set working directory to the server subdirectory of the repository
S = "${WORKDIR}/git/server"

# Specify files to install in the package
FILES:${PN} += "${bindir}/aesdsocket"
# Add required library linkages for the application
TARGET_LDFLAGS += "-pthread -lrt"

do_configure () {
    # No specific configuration required
    :
}

do_compile () {
    # Build the application using the provided Makefile
    oe_runmake
}

do_install () {
    # Create installation directory
    install -d ${D}${bindir}
    
    # Install the aesdsocket binary with executable permissions
    install -m 0755 ${S}/aesdsocket ${D}${bindir}/
    
    # Uncomment and modify if you have additional files like systemd service
    # install -d ${D}${systemd_system_unitdir}
    # install -m 0644 ${S}/aesdsocket.service ${D}${systemd_system_unitdir}/
}