#ifndef ACCESS_OK_VERSION_H
#define ACCESS_OK_VERSION_H

#include <linux/version.h>
#include <linux/uaccess.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#define access_ok_wrapper(type, addr, size) access_ok(addr, size)
#else
#define access_ok_wrapper(type, addr, size) access_ok(type, addr, size)
#endif

#endif /* ACCESS_OK_VERSION_H */
