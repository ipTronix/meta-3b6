/** ***************************************************************************
 *
 **************************************************************************** */
#ifndef __FPGA_IOCTL_H
#define __FPGA_IOCTL_H

#include <linux/types.h>
#include <linux/ioctl.h>

/**
 *  Description of the flash
 *
 *  Contains the basic flash device parameter.
 *
 *  Every parameter that distinguishes this serial flash device from any other
 *  is encoded here.
 */
typedef struct {
  uint32_t      size_in_bytes;
  uint32_t      silicon_id;
  uint32_t      page_size;
  uint32_t      number_of_blocks;
  uint32_t      block_size;
}sFpgaFlashDes, *psFpgaFlashDes;


/** **************************************************************************
*                                 IOCTL definitions
************************************************************************** */
#define FPGA_IOCTL_MAGIC 'F'

#define FPGA_IOCTL_FLASH_DES      _IOR (FPGA_IOCTL_MAGIC, 0x00, psFpgaFlashDes)

#endif /* __FPGA_IOCTL_H */
