/*
 * Copyright 2018 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
 
#include "sc.h"

#include <soc/imx8/sc/ipc.h>
#include <soc/imx8/sc/svc/pm/api.h>
#include <soc/imx8/sc/svc/misc/api.h>
 
#define FIRST_MINOR 0
#define MINOR_CNT 1
 
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

static sc_ipc_t ipc = 0; 

static int scfw_open(struct inode *i, struct file *f)
{
    return 0;
}
static int scfw_close(struct inode *i, struct file *f)
{	
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int scfw_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long scfw_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
	int ioctl_err = 0;
        uint32_t mu_id;

	/* IOCTL variables */
	sc_misc_build_info_t build_info;	
	sc_misc_ctl_t msc;
	sc_pm_res_t pm;	

	/* Open IPC channel with SCU */
        sc_err_t sc_err = SC_ERR_NONE;
        sc_err = sc_ipc_getMuID(&mu_id);
        sc_err = sc_ipc_open(&ipc, mu_id);

	/* Select IOCTL call to make */
	switch (cmd){
	/* Miscellaneous service */
	case SC_MISC_GET_CONTROL:
		if(copy_from_user(&msc, (sc_misc_ctl_t *) arg, sizeof(sc_misc_ctl_t))){
			ioctl_err = -EACCES;
		}
		msc.err = sc_misc_get_control(ipc, msc.resource, msc.ctl, &msc.val);
		if (copy_to_user((sc_misc_ctl_t *)arg, &msc, sizeof(sc_misc_ctl_t))){
                	ioctl_err = -EACCES;
            	}
            	break;
	case SC_MISC_SET_CONTROL:	
		if(copy_from_user(&msc, (sc_misc_ctl_t *) arg, sizeof(sc_misc_ctl_t))){
			ioctl_err = -EACCES;
		}
		msc.err = sc_misc_set_control(ipc, msc.resource, msc.ctl, msc.val);
		if (copy_to_user((sc_misc_ctl_t *)arg, &msc, sizeof(sc_misc_ctl_t))){
                	ioctl_err = -EACCES;
            	}
            	break;
	case SC_MISC_BUILD_INFO:	
		sc_misc_build_info(ipc, &build_info.build, &build_info.commit);
		if (copy_to_user((sc_misc_build_info_t *)arg, &build_info, sizeof(sc_misc_build_info_t))){
                	ioctl_err = -EACCES;
            	}
            	break;
	/* Power management service */
	case SC_PM_GET_POWER_MODE:	
		if(copy_from_user(&pm, (sc_pm_res_t *)arg, sizeof(sc_pm_res_t))){
			ioctl_err = -EACCES;
		}
		pm.err = sc_pm_get_resource_power_mode(ipc, pm.resource, &pm.mode);
		if (copy_to_user((sc_pm_res_t *)arg, &pm, sizeof(sc_pm_res_t))){
                	ioctl_err = -EACCES;
            	}
            	break;
	case SC_PM_SET_POWER_MODE:
		if(copy_from_user(&pm, (sc_pm_res_t *)arg, sizeof(sc_pm_res_t))){
			ioctl_err = -EACCES;
		}
		pm.err = sc_pm_set_resource_power_mode(ipc, pm.resource, pm.mode);
		if (copy_to_user((sc_pm_res_t *)arg, &pm, sizeof(sc_pm_res_t))){
                	ioctl_err = -EACCES;
            	}
            	break;
        default:
            ioctl_err = -ENOTTY;
    }

    /* Close IPC channel */
    sc_ipc_close(ipc);
    return ioctl_err;
}
 
static struct file_operations query_fops =
{
    .owner = THIS_MODULE,
    .open = scfw_open,
    .release = scfw_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = scfw_ioctl
#else
    .unlocked_ioctl = scfw_ioctl
#endif
};
 
static int __init scfw_ioctl_init(void)
{
    int ret;
    struct device *dev_ret;
 
 
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "query_ioctl")) < 0)
    {
        return ret;
    }
 
    cdev_init(&c_dev, &query_fops);
 
    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return ret;
    }
     
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "sc")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }
 
    return 0;
}
 
static void __exit scfw_ioctl_exit(void)
{
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}
 
module_init(scfw_ioctl_init);
module_exit(scfw_ioctl_exit);
 
MODULE_AUTHOR("Manuel Rodriguez");
MODULE_DESCRIPTION("System Controller ioctl() Char Driver");

