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

#if 1
#define DBG_PRINT(fmt, ...)   printk("%s:%s:"fmt, DRIVER_NAME, __func__, ##__VA_ARGS__);
#define DBG_ERROR(fmt, ...)   printk("%s:%s:ERROR:"fmt, DRIVER_NAME, __func__, ##__VA_ARGS__);
#else
#define DBG_PRINT(fmt, ...)
#define DBG_ERROR(fmt, ...)
#endif

#define DRIVER_NAME    "fpga"

#define EPCS_BASE_ADDRESS 0x00009800
#define SYS_ID_0_BASE     0x00008A00
//#define SYS_ID_1_BASE     0x00008900

#define FPGA_FLG_DEVICE_IN_USE  0x00000001

/**
 * routine to request to cvifw PCIe BAR0 address
 */
extern void __iomem* cvifw_bar0_address(void);

static struct fpga_dev *fpgaDevice;

uint8_t bitrev[] = {
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
  0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
  0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
  0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
  0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
  0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
  0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
  0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
  0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
  0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
  0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
  0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
  0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
  0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
  0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
  0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
  0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

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
/*
  file->f_mapping->backing_dev_info = fpgaDevice->dev_info;
  if (fpgaDevice->dev_info == &directly_mappable_cdev_bdi) {
    file->f_mode |= FMODE_UNSIGNED_OFFSET;
  }
*/
  fpgaDevice->flags |= FPGA_FLG_DEVICE_IN_USE;

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
  int               i;

DBG_PRINT("count %d, f_pos %lld\n", count, *f_pos);
  mutex_lock(&fpgaDevice->fops_lock);
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
    DBG_ERROR("epcs_flash_read return %d\n", ret);
    kfree(fbuf);
    ret = -EFAULT;
    goto out;
  }
  for (i=0; i<count; i++) {
    fbuf[i] = bitrev[fbuf[i]];
  }
  if(copy_to_user(buf, fbuf, count)){
    ret = -EFAULT;
    goto out;
  }
  kfree(fbuf);
  *f_pos += count;
  ret = count;
out:
  mutex_unlock(&fpgaDevice->fops_lock);
  return ret;
}

/**
 */
ssize_t fpga_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
  struct fpga_dev  *dev = file->private_data;
  loff_t            pos = *f_pos;
  int               ret;
  uint8_t          *fbuf;
  int               i;
DBG_PRINT("count %d, f_pos %lld\n", count, *f_pos);
  mutex_lock(&fpgaDevice->fops_lock);
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
DBG_PRINT("kernel buffer @%p\n", fbuf);
  if (copy_from_user(fbuf, buf, count)) {
    kfree(fbuf);
    ret = -EFAULT;
    goto out;
  }

/*{
  int i;
  printk("data from user space len %d\n", count);
  for(i=0; i<count; i++){
    if( (i & 0xF) == 0){
      printk("%08X:", i);
    }
    printk(" %02X", fbuf[i]);
    if( (i & 0xF) == 0xF){
      printk("\n");
    }
  }
  if((i & 0xF)){
    printk("\n");
  }
}*/
  for (i=0; i<count; i++) {
    fbuf[i] = bitrev[fbuf[i]];
  }
  ret = epcs_flash_write(&dev->flash, pos+0x00100000, fbuf, count);
  if(ret){
    DBG_ERROR("epcs_flash_write return %d\n", ret);
    kfree(fbuf);
    ret = -EFAULT;
    goto out;
  }
  kfree(fbuf);
  *f_pos += count;
  ret = count;
out:
  mutex_unlock(&fpgaDevice->fops_lock);
  return ret;
}

/**
 */
loff_t fpga_llseek(struct file *file, loff_t off, int whence)
{
  struct fpga_dev  *dev = file->private_data;
  loff_t            newpos;

  mutex_lock(&fpgaDevice->fops_lock);
  switch(whence){
  case  0: newpos = off;                              break;  /* SEEK_SET */
  case  1: newpos = file->f_pos + off;                break;  /* SEEK_CUR */
  case  2: newpos = (dev->flash.size_in_bytes/2)+off; break;  /* SEEK_END */
  default: return -EINVAL;                                    /* can't happen */
  }
  DBG_PRINT("off %lld, whence %d, newpos %lld\n", off, whence, newpos);
  if(newpos<0){
    mutex_unlock(&fpgaDevice->fops_lock);
    return -EINVAL;
  }
  file->f_pos = newpos;
  mutex_unlock(&fpgaDevice->fops_lock);
  return file->f_pos;
}

/**
 * IOCTL
 */
static long fpga_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  struct fpga_dev *pDrv = (struct fpga_dev *)file->private_data;
  int err = 0;
  //DBG_PRINT("cmd %08X\n", cmd);
  mutex_lock(&pDrv->fops_lock);
  switch (cmd) {
  case FPGA_IOCTL_FLASH_DES:
  {
    sFpgaFlashDes des;
    des.size_in_bytes    = pDrv->flash.size_in_bytes                  ;
    des.silicon_id       = pDrv->flash.silicon_id                     ;
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
  mutex_unlock(&pDrv->fops_lock);
  return err;
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
 * file operations structure
 */
static const struct file_operations fpga_fops = {
  .owner          = THIS_MODULE,
  .open           = fpga_open,
  .read           = fpga_read,
  .write          = fpga_write,
  .llseek         = fpga_llseek,
  .unlocked_ioctl = fpga_ioctl,
  .release        = fpga_close,
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
  //len = sprintf(buf, "%08X %08X\n", fpgaDevice->sys_id_0, fpgaDevice->sys_id_1);
  len = sprintf(buf, "%08X\n", fpgaDevice->sys_id_0);
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

//  fpgaDevice->dev_info = &directly_mappable_cdev_bdi;
  mutex_init(&fpgaDevice->fops_lock);

  err = fpga_dev_init(fpgaDevice, DRIVER_NAME);
  if( err < 0) {
    DBG_ERROR("Device Registration failed\n" );
    return -1;
  }

  // Get system ids
  {
    char *adr;
    adr = cvifw_bar0_address();
    fpgaDevice->sys_id_0 = __raw_readl(adr + SYS_ID_0_BASE);
//    fpgaDevice->sys_id_1 = __raw_readl(adr + SYS_ID_1_BASE);
  }

  // chiede il puntatore al BAR0 ad altera_cvifw
  fpgaDevice->flash.register_base = cvifw_bar0_address() + EPCS_BASE_ADDRESS;

  err = epcs_flash_init(&fpgaDevice->flash);
  DBG_PRINT("register_base @ %p, fpgaDevice->flash @ %p\n",
            fpgaDevice->flash.register_base, &fpgaDevice->flash);

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

