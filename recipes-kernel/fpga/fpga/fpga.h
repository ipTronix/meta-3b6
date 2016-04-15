/** ***************************************************************************
 *
 **************************************************************************** */

#ifndef __FPGA_PCI_H
#define __FPGA_PCI_H

#include "fpga-ioctl.h"
#include "flash.h"

/**
 *
 */
struct fpga_dev {
  struct device            *dev;
  struct backing_dev_info  *dev_info;

  dev_t                     first;         // first device number
  struct cdev               c_dev;         // character device structure
  struct class             *cl;            // device class

  uint32_t                  sys_id_0;
//  uint32_t                  sys_id_1;

  struct mutex              fops_lock;

  unsigned char             flags;

  flash_epcs_dev            flash;
};

#endif    /* __FPGA_PCI_H */
