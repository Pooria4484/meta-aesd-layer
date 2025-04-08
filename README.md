# meta-aesd-layer
meta-aesd-layer: Yocto Layer for AESD Char Driver and Socket App

This repository contains the Yocto layer `meta-aesd-layer` used for the CU ECEN AESD Final Project.

It includes:

- A Linux character device driver (`aesdchar`) with circular buffer, ioctl, and llseek support.
- A multithreaded socket server (`aesdsocket`) communicating with the char device.
- Integration recipes for both kernel modules and user-space apps.
- A custom `core-image-aesd.bb` image recipe.
- Root filesystem overlays to initialize the kernel modules at boot.

## Layer Structure

```
meta-aesd-layer/
├── conf/
├── recipes-kernel/
│   └── aesdchar/
├── recipes-apps/
│   └── aesdsocket/
├── recipes-misc-modules/
├── recipes-scull/
├── recipes-aesd-assignments/
├── rootfs_overlay/
```

## How to Use

1. Clone this repository as a submodule inside your Yocto project:

```bash
git submodule add https://github.com/Pooria4484/meta-aesd-layer yocto/meta-aesd-layer
```

2. Add this layer to `bblayers.conf`:

```bash
BBLAYERS += "${TOPDIR}/../meta-aesd-layer"
```

3. Add the packages to your image by appending the following line in your `core-image.bbappend`:

```bash
IMAGE_INSTALL:append = " aesdchar aesdsocket"
```

## Maintainer

Pouria Alaeinezhad – [final project link](https://github.com/cu-ecen-aeld/final-project-Pooria4484)
