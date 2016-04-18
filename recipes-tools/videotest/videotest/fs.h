/** ***************************************************************************
 * @file fs.h
 * @brief
 **************************************************************************** */

#ifndef __FS_H
#define __FS_H

#include <stdint.h>





#define FS_TYPE_SOCK  0x00000001
#define FS_TYPE_LNK   0x00000002
#define FS_TYPE_REG   0x00000004
#define FS_TYPE_BLK   0x00000008
#define FS_TYPE_DIR   0x00000010
#define FS_TYPE_CHR   0x00000020
#define FS_TYPE_IFO   0x00000040


int fsDirGet(char* path, char* wildc, char** res, int size);
int fsFileStat(char* name, uint32_t type);

int fsFileLoad(char* name, uint8_t** buf);

#endif /* __FS_H */
