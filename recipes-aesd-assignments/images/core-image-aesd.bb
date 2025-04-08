inherit core-image
#CORE_IMAGE_EXTRA_INSTALL += "aesd-assignments"
CORE_IMAGE_EXTRA_INSTALL += "openssh"
#IMAGE_INSTALL += "misc-modules"
inherit extrausers
# See https://docs.yoctoproject.org/singleindex.html#extrausers-bbclass
# We set a default password of root to match our busybox instance setup
# Don't do this in a production image
# PASSWD below is set to the output of
# printf "%q" $(mkpasswd -m sha256crypt root) to hash the "root" password
# string
PASSWD = "\$5\$2WoxjAdaC2\$l4aj6Is.EWkD72Vt.byhM5qRtF9HcCM/5YpbxpmvNB5"
EXTRA_USERS_PARAMS = "usermod -p '${PASSWD}' root;"

do_copy_overlay() {
    cp -r ${THISDIR}/../../rootfs_overlay/* ${IMAGE_ROOTFS}/
}

ROOTFS_POSTPROCESS_COMMAND += "do_copy_overlay; "
IMAGE_INSTALL:append = " scull"
IMAGE_INSTALL += "aesdchar"
IMAGE_INSTALL:append = " aesdsocket"
