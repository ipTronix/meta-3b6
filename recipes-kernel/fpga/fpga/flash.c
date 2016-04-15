/** ***************************************************************************
 * @file flash.c
 *
 **************************************************************************** */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/printk.h>

#include "epcs.h"
#include "flash.h"

#define MODULE_NAME "flash"
#define DBG_PRINT(fmt, ...)   printk("%s:%s:"fmt, MODULE_NAME, __func__, ##__VA_ARGS__);
#define DBG_ERROR(fmt, ...)   printk("%s:%s:ERROR:"fmt, MODULE_NAME, __func__, ##__VA_ARGS__);

static int epcs_flash_query(flash_epcs_dev* flash);

/*
 * epcs_flash_init
 *
 */
int epcs_flash_init(flash_epcs_dev* flash)
{
  int ret_code = 0;

  /* Set up function pointers and/or data structures as needed. */
  ret_code = epcs_flash_query(flash);

  DBG_PRINT("FLASH description\n"
            "four_bytes_mode   %d\n"
            "size_in_bytes     %d\n"
            "number_of_regions %d\n"
            "page_size         %d\n"
            "region_info[0].region_size      %d\n"
            "region_info[0].number_of_blocks %d\n"
            "region_info[0].block_size       %d\n"
            "region_info[0].offset           %d\n",
            flash->four_bytes_mode                ,
            flash->size_in_bytes                  ,
            flash->number_of_regions              ,
            flash->page_size                      ,
            flash->region_info[0].region_size     ,
            flash->region_info[0].number_of_blocks,
            flash->region_info[0].block_size      ,
            flash->region_info[0].offset          );

  return ret_code;
}

/**
 */
static int epcs_flash_query(flash_epcs_dev* flash)
{
  int ret_code = 0;

  /* Decide if an epcs flash device is attached.
   *
   * ret_code = -ENODEV = No device found!
   */

  /* It may be necessary to unlock or lock sectors,
   * reset the device, or whatever, to ensure that
   * it's in a known working state.
  */

  /* Disable 4-bytes address mode. */
  flash->four_bytes_mode = 0;

  /* Send the RES command sequence */
  flash->silicon_id = epcs_read_electronic_signature(flash->register_base);

  /* Fill in all device-specific parameters. */
  if (flash->silicon_id == 0x16) /* EPCS64 */
  {
    flash->region_info[0].region_size = 64 * 1024 * 1024 / 8;
    flash->region_info[0].number_of_blocks = 128;
    flash->region_info[0].block_size = 65536;
  }
  else if (flash->silicon_id == 0x14) /* EPCS16 */
  {
    flash->region_info[0].region_size = 16 * 1024 * 1024 / 8;
    flash->region_info[0].number_of_blocks = 32;
    flash->region_info[0].block_size = 65536;
  }
  else if (flash->silicon_id == 0x13) /* EPCS8 */
  {
    flash->region_info[0].region_size = 8 * 1024 * 1024 / 8;
    flash->region_info[0].number_of_blocks = 16;
    flash->region_info[0].block_size = 65536;
  }
  else if (flash->silicon_id == 0x12) /* EPCS4 */
  {
    flash->region_info[0].region_size = 4 * 1024 * 1024 / 8;
    flash->region_info[0].number_of_blocks = 8;
    flash->region_info[0].block_size = 65536;
  }
  else if (flash->silicon_id == 0x10) /* EPCS1 */
  {
    flash->region_info[0].region_size = 1 * 1024 * 1024 / 8;
    flash->region_info[0].number_of_blocks = 4;
    flash->region_info[0].block_size = 32768;
  }
  else
  {
    /*
     * Read electronic signature doesn't work for newer devices; try
     * the "Read Device ID" command" before giving up.
     */
    flash->silicon_id = epcs_read_device_id(flash->register_base);

    /*
     * Last byte is the density ID. Note the difference between
     * EPCS128 and EPCQ128 -- arranged differently, though the
     * least significant byte of each is '0x18'.
     */
    if((flash->silicon_id & 0xFFFFFF) == 0x20BA15) /* EPCQ16 */
    {
      flash->region_info[0].region_size = 16 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 32; /* number of sectors */
      flash->region_info[0].block_size = 65536;  /* sector size */
    }
    else if((flash->silicon_id & 0xFFFFFF) == 0x20BA16) /* EPCQ32 */
    {
      flash->region_info[0].region_size = 32 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 64; /* number of sectors */
      flash->region_info[0].block_size = 65536;  /* sector size */
    }
    else if((flash->silicon_id & 0xFFFFFF) == 0x20BA17) /* EPCQ64 */
    {
      flash->region_info[0].region_size = 64 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 128; /* number of sectors */
      flash->region_info[0].block_size = 65536;  /* sector size */
    }
    else if((flash->silicon_id & 0xFFFFFF) == 0x20BA18) /* EPCQ128 */
    {
      flash->region_info[0].region_size = 128 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 256; /* number of sectors */
      flash->region_info[0].block_size = 65536;  /* sector size */
    }
    else if((flash->silicon_id & 0xFF) == 0x18) /* EPCS128 */
    {
      flash->region_info[0].region_size = 128 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 64;
      flash->region_info[0].block_size = 262144;
    }
    else if((flash->silicon_id & 0xFF ) == 0x19) /* EPCQ256 */
    {
      flash->region_info[0].region_size = 256 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 512; /* number of sectors */
      flash->region_info[0].block_size = 65536;  /* sector size */

      /* Enable 4-bytes address mode if the device density is greater than 256Mbit.
       * Last byte of device ID is density ID.
       *
       * The whole 4-bytes thing extends commands that send a memory address to
       * the chip (read, write, erase) from 3 bytes to 4. The 4-byte address mode
       * must first be programmed into the device, though. To complicate things,
       * other Altera IP expects the chip to be in 3 byte address mode when they
       * start using it. To be nice, we'll place the device into 4-byte address mode
       * when we need to, and take it back out when we're done.
       */
      flash->four_bytes_mode = 1;
    }
    else if((flash->silicon_id & 0xFF ) == 0x20) /* EPCQ512 */
    {
      flash->region_info[0].region_size = 512 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 1024; /* number of sectors */
      flash->region_info[0].block_size = 65536;  /* sector size */

      /* Enable 4-bytes address mode if the device density is greater than 256Mbit. */
      flash->four_bytes_mode = 1;
    }
    else if((flash->silicon_id & 0xFF ) == 0x21) /* EPCQ1024 */
    {
      flash->region_info[0].region_size = 1024 * 1024 * 1024 / 8;
      flash->region_info[0].number_of_blocks = 2048; /* number of sectors */
      flash->region_info[0].block_size = 65536;  /* sector size */

      /* Enable 4-bytes address mode if the device density is greater than 256Mbit. */
      flash->four_bytes_mode = 1;
    }
    else
    {
      ret_code = -ENODEV; /* No known device found! */
    }
  }

  flash->size_in_bytes         = flash->region_info[0].region_size;
  flash->number_of_regions     = 1;
  flash->region_info[0].offset = 0;
  flash->page_size             = 256;

  /* Consider clearing all BP bits here. */
  return ret_code;
}

/**
 */
static int epcs_flash_memcmp(flash_epcs_dev* flash,
                             const void* src_buffer,
                             int offset, size_t n)
{
  /*
   * Compare chunks of memory at a time, for better serial-flash
   * read efficiency.
   */
  uint8_t   chunk_buffer[32];
  const int chunk_size = sizeof(chunk_buffer) / sizeof(*chunk_buffer);
  int       current_offset = 0;

//  DBG_PRINT("flash @ %p, src_buffer %p, offset %d, n %d\n",
//            flash, src_buffer, offset, n);
  while(n > 0){
    int this_chunk_size = n > chunk_size ? chunk_size : n;
    int this_chunk_cmp;

//DBG_PRINT("read from offset %08X, size %d\n",
//          offset + current_offset, this_chunk_size);
    if(epcs_flash_read(flash, offset + current_offset,
                       chunk_buffer, this_chunk_size) < 0 ){
      /*
      * If the read fails, I'm not sure what the appropriate action is.
      * Compare success seems wrong, so make it compare fail.
      */
      return -1;
    }
/*{
  int i;
  printk("readed chunk len %d\n", this_chunk_size);
  for(i=0; i<this_chunk_size; i++){
    if( (i & 0xF) == 0){
      printk("%08X:", i);
    }
    printk(" %02X", chunk_buffer[i]);
    if( (i & 0xF) == 0xF){
      printk("\n");
    }
  }
  if((i & 0xF)){
    printk("\n");
  }
}*/
    /* Compare this chunk against the source memory buffer. */
//DBG_PRINT("compare memory @%p, @%p, size %d\n",
//          &((unsigned char*)(src_buffer))[current_offset],
//          chunk_buffer, this_chunk_size);
    this_chunk_cmp = memcmp(&((unsigned char*)(src_buffer))[current_offset],
                            chunk_buffer, this_chunk_size);
    if(this_chunk_cmp){
      return this_chunk_cmp;
    }
    n -= this_chunk_size;
    current_offset += this_chunk_size;
  }

  /*
   * If execution made it to this point, compare is successful.
   */
  return 0;
}

/*
 * epcs_flash_write
 *
 * Program the data into the flash at the selected address.
 *
 * Restrictions - For now this function will program the sectors it needs,
 * if it needs to erase a sector it will. If you wish to have all the contents
 * of the sector preserved you the user need to be aware of this and read out
 * the contents of that sector and add it to the data you wish to program.
 * The reasoning here is that sectors can be very large eg. 64k which is a
 * large buffer to tie up in our programming library, when not all users will
 * want that functionality.
 */
int epcs_flash_write(flash_epcs_dev* flash, int offset,
                     const void* src_addr, int length)
{
  int         ret_code = 0;
  int         i,j;
  int         data_to_write;
  int         current_offset;

  /*
   * First and foremost which sectors are affected?
   */
  for(i=0; i<flash->number_of_regions; i++){
    /* Is it in this erase block region?*/
    if((offset >= flash->region_info[i].offset) &&
       (offset < (flash->region_info[i].offset +
      flash->region_info[i].region_size)) ){
      current_offset = flash->region_info[i].offset;

      for(j=0; j<flash->region_info[i].number_of_blocks; j++){
        if( (offset >= current_offset ) &&
            (offset < (current_offset +
            flash->region_info[i].block_size)) ){
          /*
           * Check if the contents of the block are different
           * from the data we wish to put there
           */
          data_to_write = ( current_offset + flash->region_info[i].block_size
                            - offset);
          data_to_write = MIN(data_to_write, length);

  //DBG_PRINT("epcs_flash_memcmp(src_addr %p, offset %08X, data_to_write %08X\n",
//            src_addr, offset, data_to_write);
          ret_code = epcs_flash_memcmp(flash, src_addr, offset, data_to_write);
          if(ret_code){
  //DBG_PRINT("erase_block @ %p\n", current_offset);
            ret_code = epcs_flash_erase_block(flash, current_offset);
            if(!ret_code){
  //DBG_PRINT("write_block @ %p, offset %08X, src_addr %p, data_to_write %d\n",
  //          current_offset,
  //          offset,
  //          src_addr,
  //          data_to_write);
              ret_code = epcs_flash_write_block(flash,
                                                current_offset,
                                                offset,
                                                src_addr,
                                                data_to_write);
            }
          }

          /* Was this the last block? */
          if((length == data_to_write) || ret_code){
            goto finished;
          }
          length  -= data_to_write;
          offset   = current_offset + flash->region_info[i].block_size;
          src_addr = (uint8_t*)src_addr + data_to_write;
        }
        current_offset += flash->region_info[i].block_size;
      }
    }
  }

finished:
  return ret_code;
}

/*
 *  epcs_flash_get_info
 *
 *  Pass the table of erase blocks to the user
 */
int epcs_flash_get_info(flash_epcs_dev* flash, flash_region** info,
                        int* number_of_regions)
{
  int ret_code = 0;

  *number_of_regions = flash->number_of_regions;

  if(!flash->number_of_regions){
    ret_code = -EIO;
  }else if (flash->number_of_regions > ALT_MAX_NUMBER_OF_FLASH_REGIONS){
    ret_code = -ENOMEM;
  }else{
    *info = &flash->region_info[0];
  }
  return ret_code;
}


/* This might be a candidate for optimization.  Precompute the last-address? */
static inline int epcs_test_address(flash_epcs_dev* flash, int offset)
{
  int             ret_code = 0;
  /* Error checking:
   * if the block offset is outside of the memory, return -EIO.
   */
  const uint32_t  last_region_index = flash->number_of_regions - 1;
  uint32_t        last_device_address =
    -1 +
    flash->region_info[last_region_index].offset +
    flash->region_info[last_region_index].region_size;

  if(offset > last_device_address){
    /* Someone tried to erase a block outside of this device's range. */
    ret_code = -EIO;
  }
  return ret_code;
}


/*
 *
 * Erase the selected erase block ("sector erase", from the POV
 * of the EPCS data sheet).
 */
int epcs_flash_erase_block(flash_epcs_dev* flash, int block_offset)
{
  int ret_code = 0;

  ret_code = epcs_test_address(flash, block_offset);

  if(ret_code >= 0){
    /* Send the Sector Erase command, whose 3 address bytes are anywhere
     * within the chosen sector.
     */
    epcs_sector_erase(flash->register_base, block_offset, flash->four_bytes_mode);
  }
  return ret_code;
}

/* Write, assuming that someone has kindly erased the appropriate
 * sector(s).
 * Note: "block_offset" is the base of the current erase block.
 * "data_offset" is the absolute address (from the 0-base of this
 * device's memory) of the beginning of the write-destination.
 * This device has no need for "block_offset", but it's included for
 * function type compatibility.
 */
int epcs_flash_write_block(flash_epcs_dev* flash, int block_offset,
                           int data_offset, const void* data, int length)
{
  int ret_code;
  int buffer_offset = 0;
  int length_of_current_write;

  ret_code = epcs_test_address(flash, data_offset);

  if(ret_code >= 0){
    /* "Block" writes must be broken up into the page writes that
     * the device understands.  Partial page writes are allowed.
     */
    while(length){
      int next_page_start = (data_offset + flash->page_size) & ~(flash->page_size - 1);
      length_of_current_write = MIN(length, next_page_start - data_offset);

      epcs_write_buffer(flash->register_base, data_offset,
                        &((const uint8_t*)data)[buffer_offset],
                        length_of_current_write,
          flash->four_bytes_mode);

      length -= length_of_current_write;
      buffer_offset += length_of_current_write;
      data_offset = next_page_start;
    }
  }
  return ret_code;
}

/*
 *  If you try to read beyond the end of the memory, you'll wrap around
 *  to the beginning.  Reads that start beyond the end of the memory are
 *  flagged as errors with EIO (is there a better error code?).
 */
int epcs_flash_read(flash_epcs_dev* flash, int offset,
                    void* dest_addr, int length)
{
  int ret_code = 0;

  ret_code = epcs_test_address(flash, offset);

  if(ret_code >= 0){
    ret_code = epcs_read_buffer(flash->register_base, offset, dest_addr, length,
                                flash->four_bytes_mode);

    /* epcs_read_buffer returns the number of buffers read, but
     * epcs_flash_read returns 0 on success, <0 on failure.
     */
    if(ret_code == length){
      ret_code = 0;
    }
  }
  return ret_code;
}
