/** ***************************************************************************
 * @file epcs.h
 *
 **************************************************************************** */

#ifndef __EPCS_H
#define __EPCS_H

#include <linux/types.h>

/* If you need to make multiple accesses to the same slave then you should 
 * set the merge bit in the flags for all of them except the first.
 */
#define EPCS_COMMAND_MERGE (0x01)

/*
 * If you need the slave select line to be toggled between words then you
 * should set the toggle bit in the flag.
 */
#define EPCS_COMMAND_TOGGLE_SS_N (0x02)


uint32_t epcs_read_device_id(char* base);
uint8_t  epcs_read_electronic_signature(char* base);
uint8_t  epcs_read_status_register(char* base);
void     epcs_sector_erase(char* base, uint32_t offset, uint32_t four_bytes_mode);
int32_t  epcs_read_buffer(char* base, int offset, uint8_t *dest_addr, int length,
                          uint32_t four_bytes_mode);
void     epcs_write_enable(char* base);
void     epcs_write_status_register(char* base, uint8_t value);
int32_t  epcs_write_buffer(char* base, int offset, const uint8_t* src_addr, 
                           int length, uint32_t four_bytes_mode);
void     epcs_enter_4_bytes_mode(char* base);
void     epcs_exit_4_bytes_mode(char* base);


#endif /* __EPCS_H */
