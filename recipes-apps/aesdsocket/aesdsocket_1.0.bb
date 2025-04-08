SUMMARY = "aesdsocket user-space application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://aesdsocket.c;beginline=1;endline=20;md5=a5084d5bbfbed327be1ac946f6b5e853"

SRC_URI = "file://aesdsocket.c \
           file://aesdsocket-start-stop"

S = "${WORKDIR}"

inherit update-rc.d

INITSCRIPT_NAME = "aesdsocket"
INITSCRIPT_PARAMS = "defaults"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o aesdsocket aesdsocket.c
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 aesdsocket ${D}${bindir}/aesdsocket

    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/aesdsocket-start-stop ${D}${sysconfdir}/init.d/aesdsocket
}
