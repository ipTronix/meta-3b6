/** ***************************************************************************
 *                            FPGA FLASH Programmer                           *
 **************************************************************************** */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "fpga.h"
#include "fpga-ioctl.h"

#define DBG_PRINT(fmt, ...)   printk("%s:%s:"fmt, DRIVER_NAME, __func__, ##__VA_ARGS__);
#define DBG_ERROR(fmt, ...)   printk("%s:%s:ERROR:"fmt, DRIVER_NAME, __func__, ##__VA_ARGS__);

#define DRIVER_NAME    "fpga"

#define EPCS_BASE_ADDRESS 0x00009800
#define SYS_ID_0_BASE     0x00004480
#define SYS_ID_1_BASE     0x00008900

#define FPGA_FLG_DEVICE_IN_USE  0x00000001

/**
 * routine to request to cvifw PCIe BAR0 address
 */
extern void __iomem* cvifw_bar0_address(void);

static struct fpga_dev *fpgaDevice;

/** ***************************************************************************
 *                               File Operations                              *
 **************************************************************************** */

/**
 * Open
 */
static int fpga_open(struct inode *inode, struct file *file)
{
  if (fpgaDevice->flags & FPGA_FLG_DEVICE_IN_USE) {
    DBG_ERROR("Device in use\n");
    return -EBUSY;
  }

  mutex_lock(&fpgaDevice->fops_lock);

  file->private_data = fpgaDevice;

  file->f_mapping->backing_dev_info = fpgaDevice->dev_info;
  if (fpgaDevice->dev_info == &directly_mappable_cdev_bdi) {
    file->f_mode |= FMODE_UNSIGNED_OFFSET;
  }
  fpgaDevice->flags |=  FPGA_FLG_DEVICE_IN_USE;

  mutex_unlock(&fpgaDevice->fops_lock);

  return 0;
}

/**
 */
ssize_t fpga_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
  struct fpga_dev  *dev = file->private_data;
  loff_t            pos = *f_pos;
  int               ret;
  uint8_t          *fbuf;
/* TODO
  if(down_interruptible(&dev->sem)){
    return -ERESTARTSYS;
  }
 */
  if( pos >= (dev->flash.size_in_bytes/2) ){
    ret = -EFAULT;
    goto out;
  }
  if( (pos+count) > (dev->flash.size_in_bytes/2) ){
    count = (dev->flash.size_in_bytes/2) - pos;
  }
  fbuf = (uint8_t*)kmalloc(count, GFP_KERNEL);
  if(!fbuf){
    ret = -EFAULT;
    goto out;
  }
  ret = epcs_flash_read(&dev->flash, pos+0x00100000, fbuf, count);
  if(ret){
    kfree(fbuf);
    ret = -EFAULT;
    goto out;
    //errore lettura da flash
  }
  if(copy_to_user(buf, fbuf, count)){
    ret = -EFAULT;
    goto out;
  }
  kfree(fbuf);
  *f_pos += count;
  ret = count;
out:
  //up(&dev->sem);  TODO
  return ret;
}

/**
 */
ssize_t fpga_write(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
  struct fpga_dev  *dev = file->private_data;
  loff_t            pos = *f_pos;
  int               ret;
  uint8_t          *fbuf;

/* TODO mutex */
  if( pos >= (dev->flash.size_in_bytes/2) ){
    ret = -EFAULT;
    goto out;
  }
  if( (pos+count) > (dev->flash.size_in_bytes/2) ){
    count = (dev->flash.size_in_bytes/2) - pos;
  }
  fbuf = (uint8_t*)kmalloc(count, GFP_KERNEL);
  if(!fbuf){
    ret = -EFAULT;
    goto out;
  }
  if (copy_from_user(fbuf, buf, count)) {
    kfree(fbuf);
    ret = -EFAULT;
    goto out;
  }
  ret = epcs_flash_write(&dev->flash, pos+0x00100000, fbuf, count);
  if(ret){
    kfree(fbuf);
    ret = -EFAULT;
    goto out;
    //errore lettura da flash
  }
  kfree(fbuf);
  *f_pos += count;
  ret = count;
out:
/* TODO mutex */
  return ret;
}

/**
 */
loff_t fpga_llseek(struct file *file, loff_t off, int whence)
{
  struct fpga_dev  *dev = file->private_data;
  loff_t            newpos;

  switch(whence){
  case  0: newpos = off;                              break;  /* SEEK_SET */
  case  1: newpos = file->f_pos + off;                break;  /* SEEK_CUR */
  case  2: newpos = (dev->flash.size_in_bytes/2)+off; break;  /* SEEK_END */
  default: return -EINVAL;                                    /* can't happen */
  }
  if(newpos<0){
    return -EINVAL;
  }
  file->f_pos = newpos;
  return file->f_pos;
}

/**
 * Close
 */
static int fpga_close(struct inode *inode, struct file *file)
{
  struct fpga_dev *pDrv = (struct fpga_dev *)file->private_data;

  mutex_lock(&pDrv->fops_lock);
  pDrv->flags &= ~FPGA_FLG_DEVICE_IN_USE;
  mutex_unlock(&pDrv->fops_lock);
  return 0;
}

/**
 * IOCTL
 */
static long fpga_ioctl(struct file *file,
                       unsigned int cmd, unsigned long arg)
{
  struct fpga_dev *pDrv = (struct fpga_dev *)file->private_data;
  int err = 0;
  //DBG_PRINT("cmd %08X\n", cmd);
  //mutex_lock(&pDrv->fops_lock);
  switch (cmd) {
  case FPGA_IOCTL_FLASH_DES:
  {
    sFpgaFlashDes des;
    des.size_in_bytes    = pDrv->flash.size_in_bytes                  ;
    des.silicon_id       = pDrv->flash.silicon_id;
    des.page_size        = pDrv->flash.page_size                      ;
    des.number_of_blocks = pDrv->flash.region_info[0].number_of_blocks;
    des.block_size       = pDrv->flash.region_info[0].block_size      ;
    err = copy_to_user((void __user *)arg, (const void*)&des, sizeof(sFpgaFlashDes));
  }
    break;
  default:
    DBG_PRINT("IOCTL undefined\n");
    err = -EINVAL;
  }
  //mutex_unlock(&pDrv->fops_lock);
  return err;
}

/**
 * file operations structure
 */
static const struct file_operations fpga_fops = {
  .owner          = THIS_MODULE,
  .open           = fpga_open,
  .llseek         = fpga_llseek,
  .read           = fpga_read,
  .write          = fpga_write,
  .release        = fpga_close,
  .unlocked_ioctl = fpga_ioctl,
};

/** ***************************************************************************
 *                               sys fs entries                               *
 **************************************************************************** */

/**
 *
 */
static ssize_t sysid_r(struct device* dev, struct device_attribute* attr, char* buf)
{
  ssize_t   len;

  //mutex_lock(&sysfs_lock);  TODO
  len = sprintf(buf, "%08X %08X\n", fpgaDevice->sys_id_0, fpgaDevice->sys_id_1);
  //mutex_unlock(&sysfs_lock);
  return len;
}

/**
 * Declare the sysfs entries. The macros create instances of:
 *    dev_attr_sysid
 */
static DEVICE_ATTR(sysid, S_IRUGO, sysid_r, NULL);

/** ***************************************************************************
 *                               device creation                              *
 **************************************************************************** */

/**
 * fpga_dev_init
 * char device creation
 */
static int fpga_dev_init(struct fpga_dev* dev, char* name)
{
  int err;

  DBG_PRINT("Device Registration\n");
  err = alloc_chrdev_region(&dev->first, 0, 1, name);
  if ( err < 0) {
    DBG_ERROR("Device Registration failed\n");
    return -1;
  }

  DBG_PRINT("Class creation\n");
  dev->cl = class_create(THIS_MODULE, "fpga");
  if ( dev->cl == NULL ) {
    DBG_ERROR("Class creation failed\n");
    unregister_chrdev_region(dev->first, 1);
    return -1;
  }

  DBG_PRINT("Device creation\n");
  dev->dev = device_create(dev->cl, NULL, dev->first, NULL, name);
  if (IS_ERR(dev->dev)) {
    DBG_ERROR("Device creation failed\n");
    class_destroy(dev->cl);
    unregister_chrdev_region(dev->first, 1);
    return -1;
  }

  DBG_PRINT("Device addition\n");
  cdev_init(&dev->c_dev, &fpga_fops);
  if( cdev_add(&dev->c_dev, dev->first, 1) == -1) {
    DBG_ERROR("Device addition failed\n");
    device_destroy(dev->cl, dev->first);
    class_destroy(dev->cl);
    unregister_chrdev_region(dev->first, 1);
    return -1;
  }
  dev->dev->dma_mask = &dev->dev->coherent_dma_mask;
  dev->dev->coherent_dma_mask = DMA_BIT_MASK(32);

  DBG_PRINT("sys fs entry creation\n");
  err = device_create_file(dev->dev, &dev_attr_sysid);
  if (err) {
    DBG_ERROR("sys fs entry creation failed\n");
    device_destroy(dev->cl, dev->first);
    class_destroy(dev->cl);
    unregister_chrdev_region(dev->first, 1);
    return -1;
  }

  return 0;
}

/** **************************************************************************
 * fpga_dev_destroy
 * char device destroy
 ************************************************************************** */
int fpga_dev_destroy(struct fpga_dev* dev)
{
  device_remove_file(dev->dev, &dev_attr_sysid);
  cdev_del(&dev->c_dev);
  device_destroy(dev->cl, dev->first);
  class_destroy(dev->cl);
  unregister_chrdev_region(dev->first, 1);

  return 0;
}

/** ***************************************************************************
 *
 **************************************************************************** */
/*
static const struct of_device_id fpga_dt_ids[] = {
  { .compatible = "elt,pdmg" },
  {},
};

MODULE_DEVICE_TABLE(of, fpga_dt_ids);

static struct platform_driver fpga_driver = {
  .driver = {
    .name           = "fpga",
    .owner          = THIS_MODULE,
    .of_match_table = fpga_dt_ids,
  },
};
*/

/**
 * Module Init
 */
static int __init fpga_init(void)
{
  int err;

  DBG_PRINT("Registering %s driver\n", DRIVER_NAME);

  fpgaDevice = kzalloc(sizeof(struct fpga_dev), GFP_KERNEL);
  if (!fpgaDevice) {
    DBG_ERROR("not enough memory\n");
    return -ENOMEM;
  }

  fpgaDevice->dev_info = &directly_mappable_cdev_bdi;
  mutex_init(&fpgaDevice->fops_lock);

  err = fpga_dev_init(fpgaDevice, DRIVER_NAME);
  if( err < 0) {
    DBG_ERROR("Device Registration failed\n" );
    return -1;
  }

  // Get system ids
  {
    char* adr;
    adr = cvifw_bar0_address();
    fpgaDevice->sys_id_0 = __raw_readl(adr + SYS_ID_0_BASE);
    fpgaDevice->sys_id_1 = __raw_readl(adr + SYS_ID_1_BASE);
  }

  // TODO chiede il puntewtore al BAR0 a altera_cvifw
  fpgaDevice->flash.register_base = cvifw_bar0_address() + EPCS_BASE_ADDRESS;
  err = epcs_flash_init(&fpgaDevice->flash);



  DBG_PRINT("Driver allocated major:%d minor:%d\n",
            MAJOR(fpgaDevice->first), MINOR(fpgaDevice->first));

  return 0;
}

/**
 * Module Exit
 */
static void __exit fpga_exit(void)
{
  mutex_destroy(&fpgaDevice->fops_lock);
  fpga_dev_destroy(fpgaDevice);
  kfree(fpgaDevice);
  DBG_PRINT("Driver unregistered\n");
}

module_init(fpga_init);
module_exit(fpga_exit);

MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_AUTHOR("IpTronix S.r.l. <info@iptronix.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");

