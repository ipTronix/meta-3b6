/** ***************************************************************************
 *
 **************************************************************************** */

#ifndef __FLASH_H
#define __FLASH_H

#include <linux/types.h>


#ifndef MIN
#define MIN(a,b) (((a)<(b))? (a): (b))
#endif

/*
 * Description of a single Erase region
 */
typedef struct flash_region {
  int   offset;
  int   region_size;
  int   number_of_blocks;
  int   block_size;
}flash_region;

#define ALT_MAX_NUMBER_OF_FLASH_REGIONS 8

/*
 *  Description of the flash
 *
 *  Contains the basic alt_flash_dev, plus
 *  epcs-specific parameters.
 *
 *  Every parameter that distinguishes this
 *  serial flash device from any other is
 *  encoded here.
 *
 *  Example:
 *  size_in_bytes: the number of bytes of
 *  storage in this chip.
 */
typedef struct flash_epcs_dev {
  int           number_of_regions;
  flash_region  region_info[ALT_MAX_NUMBER_OF_FLASH_REGIONS];

  char         *register_base;
  uint32_t      size_in_bytes;
  uint32_t      silicon_id;
  uint32_t      page_size;
  uint32_t      four_bytes_mode;
}flash_epcs_dev;

int epcs_flash_init(flash_epcs_dev* flash);
int epcs_flash_read(flash_epcs_dev* flash, int offset,
                    void* dest_addr, int length);
int epcs_flash_write(flash_epcs_dev* flash, int offset,
                     const void* src_addr, int length);

int epcs_flash_erase_block(flash_epcs_dev* flash, int block_offset);
int epcs_flash_write_block(flash_epcs_dev* flash, int block_offset,
                               int data_offset, const void* data, int length);


#endif /* __FLASH_H */
