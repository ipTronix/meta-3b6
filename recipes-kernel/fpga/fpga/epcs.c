/** ***************************************************************************
 * @file epcs.c
 *
 **************************************************************************** */
#include <linux/io.h>
#include <linux/printk.h>

#include "epcs.h"

#define MODULE_NAME "epcs"
#define DBG_PRINT(fmt, ...)   printk("%s:%s:"fmt, MODULE_NAME, __func__, ##__VA_ARGS__);
#define DBG_ERROR(fmt, ...)   printk("%s:%s:ERROR:"fmt, MODULE_NAME, __func__, ##__VA_ARGS__);


/**
 * EPCS Serial Flash Controller Core Register Map
 *  Offset          Register Name       R/W Boot Loader Code
 *  0x000 - 0x3FF   Boot ROM Memory     R
 *  0x400           Read Data           R
 *  0x404           Write Data          W
 *  0x408           Status              R/W
 *  0x40C           Control             R/W
 *  0x410           Reserved            —
 *  0x414           Slave Enable        R/W
 *  0x418           End of Packet       R/W
 */
#define EPCS_REG_RXDATA     0x400   /// Read Data           R
#define EPCS_REG_TXDATA     0x404   /// Write Data          W
#define EPCS_REG_STATUS     0x408   /// Status              R/W
#define   EPCS_STATUS_ROE_MSK              (0x8)
#define   EPCS_STATUS_ROE_OFST             (3)
#define   EPCS_STATUS_TOE_MSK              (0x10)
#define   EPCS_STATUS_TOE_OFST             (4)
#define   EPCS_STATUS_TMT_MSK              (0x20)
#define   EPCS_STATUS_TMT_OFST             (5)
#define   EPCS_STATUS_TRDY_MSK             (0x40)
#define   EPCS_STATUS_TRDY_OFST            (6)
#define   EPCS_STATUS_RRDY_MSK             (0x80)
#define   EPCS_STATUS_RRDY_OFST            (7)
#define   EPCS_STATUS_E_MSK                (0x100)
#define   EPCS_STATUS_E_OFST               (8)

#define EPCS_REG_CONTROL    0x40C   /// Control             R/W
#define   EPCS_CONTROL_IROE_MSK            (0x8)
#define   EPCS_CONTROL_IROE_OFST           (3)
#define   EPCS_CONTROL_ITOE_MSK            (0x10)
#define   EPCS_CONTROL_ITOE_OFST           (4)
#define   EPCS_CONTROL_ITRDY_MSK           (0x40)
#define   EPCS_CONTROL_ITRDY_OFS           (6)
#define   EPCS_CONTROL_IRRDY_MSK           (0x80)
#define   EPCS_CONTROL_IRRDY_OFS           (7)
#define   EPCS_CONTROL_IE_MSK              (0x100)
#define   EPCS_CONTROL_IE_OFST             (8)
#define   EPCS_CONTROL_SSO_MSK             (0x400)
#define   EPCS_CONTROL_SSO_OFST            (10)
#define EPCS_REG_RES__01    0x410   /// Reserved            —
#define EPCS_REG_SLAVE_S    0x414   /// Slave Enable        R/W
#define EPCS_REG_EOP        0x418   /// End of Packet       R/W

/**
 */
#define epcs_read    0x03
#define epcs_pp      0x02
#define epcs_wren    0x06
#define epcs_wrdi    0x04
#define epcs_rdsr    0x05
#define epcs_wrsr    0x01
#define epcs_se      0xD8
#define epcs_be      0xC7
#define epcs_dp      0xB9
#define epcs_res     0xAB
#define epcs_rdid    0x9F
#define epcs_en4b    0xB7    /* Enter 4-byte mode */
#define epcs_dis4b   0xE9    /* Exit 4-byte mode */


#define EPCS_CMD_RDY_TIMEOUT    100000
#define EPCS_CMD_TX_TIMEOUT     100000

/**
 */
int epcs_command(char* base, uint32_t slave,
                 uint32_t write_length, const uint8_t * write_data,
                 uint32_t read_length, uint8_t * read_data,
                 uint32_t flags)
{
  const uint8_t  *write_end   = write_data + write_length;
  uint8_t        *read_end    = read_data + read_length;
  uint32_t        write_zeros = read_length;
  uint32_t        read_ignore = write_length;
  uint32_t        status;

  /* We must not send more than two bytes to the target before it has
   * returned any as otherwise it will overflow. */
  /* Unfortunately the hardware does not seem to work with credits > 1,
   * leave it at 1 for now. */
  uint32_t        credits = 1;
  int             ret_code = read_length;
  int             i;
  /* Warning: this function is not currently safe if called in a multi-threaded
   * environment, something above must perform locking to make it safe if more
   * than one thread intends to use it.
   */
  __raw_writel(1 << slave, base + EPCS_REG_SLAVE_S);

  /* Set the SSO bit (force chipselect) only if the toggle flag is not set */
  if ((flags & EPCS_COMMAND_TOGGLE_SS_N) == 0) {
    __raw_writel(EPCS_CONTROL_SSO_MSK, base + EPCS_REG_CONTROL);
  }

  /*
   * Discard any stale data present in the RXDATA register, in case
   * previous communication was interrupted and stale data was left
   * behind.
   */
  __raw_readl(base + EPCS_REG_RXDATA);

  /* Keep clocking until all the data has been processed. */
  for(;;){
/*
    do{
      status = __raw_readl(base + EPCS_REG_STATUS);
    }while(((status & EPCS_STATUS_TRDY_MSK) == 0 || credits == 0) &&
            (status & EPCS_STATUS_RRDY_MSK) == 0);
*/
    for(i=0; i<EPCS_CMD_RDY_TIMEOUT; i++){
      status = __raw_readl(base + EPCS_REG_STATUS);
      if( !(((status & EPCS_STATUS_TRDY_MSK) == 0 || credits == 0) &&
             (status & EPCS_STATUS_RRDY_MSK) == 0) ){
        break;
      }
    }
    if(i>=EPCS_CMD_RDY_TIMEOUT){
      DBG_ERROR("CMD_RDY_TIMEOUT\n");
      ret_code = -1;
      break;
    }
/**/

    if( ((status & EPCS_STATUS_TRDY_MSK) != 0) && (credits > 0) ){
      credits--;

      if(write_data < write_end){
        __raw_writel(*write_data++, base + EPCS_REG_TXDATA);
      }else if (write_zeros > 0){
        write_zeros--;
        __raw_writel(0, base + EPCS_REG_TXDATA);
      }else{
        credits = -1024;
      }
    }

    if( (status & EPCS_STATUS_RRDY_MSK) != 0 ){
      uint32_t rxdata = __raw_readl(base + EPCS_REG_RXDATA);

      if(read_ignore > 0){
        read_ignore--;
      }else{
        *read_data++ = (uint8_t)rxdata;
      }
      credits++;

      if( (read_ignore == 0) && (read_data == read_end) ){
        break;
      }
    }
  }

  /* Wait until the interface has finished transmitting */
/*
  do{
    status = __raw_readl(base + EPCS_REG_STATUS);
  }while((status & EPCS_STATUS_TMT_MSK) == 0);
*/
  for(i=0; i<EPCS_CMD_TX_TIMEOUT; i++){
    status = __raw_readl(base + EPCS_REG_STATUS);
    if( !((status & EPCS_STATUS_TMT_MSK) == 0) ){
      break;
    }
  }
  if(i>=EPCS_CMD_TX_TIMEOUT){
    DBG_ERROR("CMD_TX_TIMEOUT\n");
    ret_code = -1;
  }
/**/

  /* Clear SSO (release chipselect) unless the caller is going to
   * keep using this chip
   */
  if((flags & EPCS_COMMAND_MERGE) == 0){
    __raw_writel(0, base + EPCS_REG_CONTROL);
  }
  //return read_length;

  return ret_code;
}

uint8_t epcs_read_status_register(char* base)
{
  const uint8_t rdsr = epcs_rdsr;
  uint8_t status;

  epcs_command(base, 0, 1, &rdsr, 1, &status, 0);
  return status;
}

static inline int epcs_test_wip(char* base)
{
  return epcs_read_status_register(base) & 1;
}

static inline void epcs_await_wip_released(char* base)
{
  /* Wait until the WIP bit goes low. */
  while (epcs_test_wip(base))
  {
  }
}

void epcs_sector_erase(char* base, uint32_t offset, uint32_t four_bytes_mode)
{
  uint8_t se[5];
  uint8_t len;

  if(four_bytes_mode){
    se[0] = epcs_se;  /* Note: Use epcs_se for Micron EPCS256 */
    se[1] = (offset >> 24) & 0xFF;
    se[2] = (offset >> 16) & 0xFF;
    se[3] = (offset >> 8) & 0xFF;
    se[4] = offset & 0xFF;
    len   = 5;
    epcs_enter_4_bytes_mode(base);
  }else{
    se[0] = epcs_se;
    se[1] = (offset >> 16) & 0xFF;
    se[2] = (offset >> 8) & 0xFF;
    se[3] = offset & 0xFF;
    len   = 4;
  }

  /* Execute a WREN instruction */
  epcs_write_enable(base);

  epcs_command(base, 0, len, se, 0, (uint8_t*)0, 0);

  epcs_await_wip_released(base);

  if(four_bytes_mode){
    epcs_exit_4_bytes_mode(base);
  }
}

int32_t epcs_read_buffer(char* base, int offset, uint8_t *dest_addr, int length,
                        uint32_t four_bytes_mode)
{
  uint8_t   read_command[5];
  uint32_t  cmd_len;
  int32_t   ret_code = length;

  read_command[0] = epcs_read;

  if(four_bytes_mode){
    read_command[1] = (offset >> 24) & 0xFF;
    read_command[2] = (offset >> 16) & 0xFF;
    read_command[3] = (offset >> 8) & 0xFF;
    read_command[4] = offset & 0xFF;
    cmd_len = 5;
    epcs_enter_4_bytes_mode(base);
  }else{
    read_command[1] = (offset >> 16) & 0xFF;
    read_command[2] = (offset >> 8) & 0xFF;
    read_command[3] = offset & 0xFF;
    cmd_len = 4;
  }

  epcs_await_wip_released(base);
//DBG_PRINT("wip_released, send read command, dest_addr %p\n", dest_addr);
  ret_code = epcs_command(base, 0, cmd_len, read_command, length, (uint8_t*)dest_addr, 0);
//DBG_PRINT("readed %d bytes @ %p ret_code %d\n", length, offset, ret_code);

  if(four_bytes_mode){
    epcs_exit_4_bytes_mode(base);
  }

//  return length;
  return ret_code;
}

void epcs_write_enable(char* base)
{
  const uint8_t wren = epcs_wren;

  epcs_command(base, 0, 1, &wren, 0, (uint8_t*)0, 0);
}

void epcs_write_status_register(char* base, uint8_t value)
{
  uint8_t wrsr[2];

  wrsr[0] = epcs_wrsr;
  wrsr[1] = value;

  epcs_command(base, 0, 2, wrsr, 0, (uint8_t*)0, 0);

  epcs_await_wip_released(base);
}

/* Write a partial or full page, assuming that page has been erased */
int32_t epcs_write_buffer(char* base, int offset, const uint8_t* src_addr,
                          int length, uint32_t four_bytes_mode)
{
  uint8_t   pp[5];
  uint32_t  cmd_len;

  pp[0] = epcs_pp;

  if(four_bytes_mode){
    pp[1] = (offset >> 24) & 0xFF;
    pp[2] = (offset >> 16) & 0xFF;
    pp[3] = (offset >> 8) & 0xFF;
    pp[4] = offset & 0xFF;
    cmd_len = 5;
    epcs_enter_4_bytes_mode(base);
  }else{
    pp[1] = (offset >> 16) & 0xFF;
    pp[2] = (offset >> 8) & 0xFF;
    pp[3] = offset & 0xFF;
    cmd_len = 4;
  }

  /* First, WREN */
  epcs_write_enable(base);

  /* Send the PP command */
  epcs_command(base, 0, cmd_len, pp, 0, (uint8_t*)0, EPCS_COMMAND_MERGE);

  /* Send the user's buffer */
  epcs_command(base, 0, length, src_addr, 0, (uint8_t*)0, 0);

  /* Wait until the write is done.  This could be optimized -
   * if the user's going to go off and ignore the flash for
   * a while, its writes could occur in parallel with user code
   * execution.  Unfortunately, I have to guard all reads/writes
   * with wip-tests, to make that happen.
   */
  epcs_await_wip_released(base);

  if(four_bytes_mode){
    epcs_exit_4_bytes_mode(base);
  }
  return length;
}

uint8_t epcs_read_electronic_signature(char* base)
{
  const uint8_t res_cmd[] = {epcs_res, 0, 0, 0};
  uint8_t res;

  epcs_command(base, 0, sizeof(res_cmd)/sizeof(*res_cmd), res_cmd, 1, &res, 0);
  return res;
}

uint32_t epcs_read_device_id(char* base)
{
  const uint8_t rd_id_cmd[] = {epcs_rdid};
  uint8_t id[3];

  epcs_command(base, 0, sizeof(rd_id_cmd)/sizeof(*rd_id_cmd), rd_id_cmd, 3, id, 0);
  return (uint32_t) ((id[0] << 16) | (id[1] << 8) | id[2]);
}

void epcs_enter_4_bytes_mode(char* base)
{
  const uint8_t en4b_cmd = epcs_en4b;

  /* First, WREN */
  epcs_write_enable(base);
  epcs_command(base, 0, sizeof(en4b_cmd), &en4b_cmd, 0, (uint8_t*)0, 0);
  return;
}

void epcs_exit_4_bytes_mode(char* base)
{
  const uint8_t exit4b_cmd = epcs_dis4b;

  /* First, WREN */
  epcs_write_enable(base);
  epcs_command(base, 0, sizeof(exit4b_cmd), &exit4b_cmd, 0, (uint8_t*)0, 0);
  return;
}

