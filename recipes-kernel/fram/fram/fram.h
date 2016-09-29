/** ***************************************************************************
 *
 **************************************************************************** */

#ifndef __FPGA_PCI_H
#define __FPGA_PCI_H

/**
 *
 */
typedef struct {
  size_t    size;
}sFramChip, *psFramChip;

/**
 *
 */
struct fram_dev {
  struct spi_device    *spi_dev;
  sFramChip             fram;
  uint8_t              *spi_tx_buf;
  uint8_t              *spi_rx_buf;

  struct mtd_info       mtd;

};

#endif    /* __FPGA_PCI_H */
