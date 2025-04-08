#ifndef PROC_OPS_VERSION_H
#define PROC_OPS_VERSION_H

#include <linux/version.h>
#include <linux/fs.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
#define SCULL_PROC_OPS struct proc_ops
#define SCULL_PROC_OPS_OWNER(x) .proc_ops = x
#else
#define SCULL_PROC_OPS struct file_operations
#define SCULL_PROC_OPS_OWNER(x) .proc_fops = x
#endif

#endif /* PROC_OPS_VERSION_H */
