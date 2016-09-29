/** ***************************************************************************
 *                        F-RAM FM25W 256-G SPI driver                        *
 **************************************************************************** */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/spi/spi.h>
#include <asm/uaccess.h>

#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/ioctl.h>
#include <linux/fs.h>

#include <linux/mtd/mtd.h>

#include "fram.h"
//#include "fram-ioctl.h"

#if 1
#define DBG_PRINT(fmt, ...)   printk("%s:%s:"fmt, DRIVER_NAME, __func__, ##__VA_ARGS__);
#define DBG_ERROR(fmt, ...)   printk("%s:%s:ERROR:"fmt, DRIVER_NAME, __func__, ##__VA_ARGS__);
#else
#define DBG_PRINT(fmt, ...)
#define DBG_ERROR(fmt, ...)
#endif

#define DRIVER_NAME    "fram"

#define SPI_TRANSFER_BUF_LEN    (32*1024+4)


static struct fram_dev *framDevice;

/** ***************************************************************************
 *                      F-RAM FM25W256-G SPI operations                       *
 *
 * Command Structure
 *  There are six commands, called opcodes, that can be issued by the bus
 *  master to the FM25W256.
 *  They are listed in foolowing Table.
 *  These opcodes control the functions performed by the memory.
 *    Name    Description               Opcode
 *    WREN    Set write enable latch    0000 0110b
 *    WRDI    Write disable             0000 0100b
 *    RDSR    Read Status Register      0000 0101b
 *    WRSR    Write Status Register     0000 0001b
 *    READ    Read memory data          0000 0011b
 *    WRITE   Write memory data         0000 0010b
 *
 **************************************************************************** */
#define FM25W256_WREN    0x06
#define FM25W256_WRDI    0x04
#define FM25W256_RDSR    0x05
#define FM25W256_WRSR    0x01
#define FM25W256_READ    0x03
#define FM25W256_WRITE   0x02

/**
 */
static int spi_trx(struct spi_device *spi, int len)
{
  struct spi_message  m;
  struct spi_transfer t = {
    .tx_buf        = framDevice->spi_tx_buf,
    .rx_buf        = framDevice->spi_rx_buf,
    .len           = len,
    .cs_change     = 0,
    .bits_per_word = 8,
  };
  int ret;

  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  ret = spi_sync(spi, &m);
  if(ret){
    DBG_ERROR("spi transfer failed: ret = %d\n", ret);
    return -1;
  }
  return 0;
}

/** ***************************************************************************
 *                       MTD read and write operations                        *
 **************************************************************************** */

/**
 */
static int fram_mtd_read(struct mtd_info *mtd, loff_t from,
                         size_t len, size_t *retlen, u_char *buf)
{
  int ret;

  framDevice->spi_tx_buf[0] = FM25W256_READ;
  framDevice->spi_tx_buf[1] = (from>>8)&0x7F;
  framDevice->spi_tx_buf[2] = (from   )&0xFF;

  ret = spi_trx(framDevice->spi_dev, 3+len);
  if(ret){
    DBG_ERROR("spi transfer failed: ret = %d\n", ret);
    return -1;
  }
  memcpy(buf, framDevice->spi_rx_buf+3, len);

  *retlen = len;
  return 0;
}

/**
 */
static int fram_mtd_write(struct mtd_info *mtd, loff_t to,
                          size_t len, size_t *retlen, const u_char *buf)
{
  int ret;

  framDevice->spi_tx_buf[0] = FM25W256_WREN;
  ret = spi_trx(framDevice->spi_dev, 1);
  if(ret){
    DBG_ERROR("spi transfer failed: ret = %d\n", ret);
    return -1;
  }

  framDevice->spi_tx_buf[0] = FM25W256_WRITE;
  framDevice->spi_tx_buf[1] = (to>>8)&0x7F;
  framDevice->spi_tx_buf[2] = (to   )&0xFF;
  memcpy(framDevice->spi_tx_buf+3, buf, len);
  ret = spi_trx(framDevice->spi_dev, 3+len);
  if(ret){
    DBG_ERROR("spi transfer failed: ret = %d\n", ret);
    return -1;
  }

  *retlen = len;
  return 0;
}

/**
 * Module Probe
 */
static int fram_probe(struct spi_device *spi)
{
  int ret;

  DBG_PRINT("Registering %s driver\n", DRIVER_NAME);

  framDevice = kzalloc(sizeof(struct fram_dev), GFP_KERNEL);
  if (!framDevice) {
    DBG_ERROR("not enough memory\n");
    return -ENOMEM;
  }
  framDevice->spi_dev = spi;
  framDevice->fram.size = 32*1024;

  framDevice->spi_tx_buf = devm_kzalloc(&spi->dev, SPI_TRANSFER_BUF_LEN,
                                        GFP_KERNEL);
  if (!framDevice->spi_tx_buf) {
    ret = -ENOMEM;
    kfree(framDevice);
    return -1;
  }
  framDevice->spi_rx_buf = devm_kzalloc(&spi->dev, SPI_TRANSFER_BUF_LEN,
                                        GFP_KERNEL);
  if (!framDevice->spi_rx_buf) {
    ret = -ENOMEM;
    kfree(framDevice);
    return -1;
  }

/**/
  framDevice->spi_tx_buf[0] = FM25W256_RDSR;
  ret = spi_trx(framDevice->spi_dev, 1+1);
  if(ret){
    DBG_ERROR("spi transfer failed: ret = %d\n", ret);
    kfree(framDevice);
    return -1;
  }
  DBG_PRINT("FRAM status is %02X\n", framDevice->spi_rx_buf[1]);
/**/

  framDevice->mtd.type      = MTD_RAM;
  framDevice->mtd.flags     = MTD_CAP_RAM;
  framDevice->mtd.size      = framDevice->fram.size;
  framDevice->mtd.name      = "F-RAM";
  framDevice->mtd.owner     = THIS_MODULE;
  framDevice->mtd._read     = fram_mtd_read;
  framDevice->mtd._write    = fram_mtd_write;
  framDevice->mtd.writesize = 1;
  ret = mtd_device_register(&framDevice->mtd, NULL, 0);
  if (ret) {
    DBG_ERROR("mtd_device_parse_register failed: ret = %d\n", ret);
    kfree(framDevice);
    return -1;
  }

  return 0;
}

/**
 * Module Remove
 */
static int fram_remove(void)
{
  DBG_PRINT("Driver fram_remove\n");

  /* Clean up MTD stuff. */
  //ret =
  mtd_device_unregister(&framDevice->mtd);

  kfree(framDevice);
  return 0;
}

static struct of_device_id fram_of_match[] = {
  { .compatible = "Cypress,FM25W256" },
  { }
};

static struct spi_driver fram_driver = {
  .probe    = fram_probe,
  .remove   = fram_remove,
  .driver = {
    .name           = DRIVER_NAME,
    .owner          = THIS_MODULE,
    .of_match_table = fram_of_match,
  },
};

module_spi_driver(fram_driver);

MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_AUTHOR("IpTronix S.r.l. <info@iptronix.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");

