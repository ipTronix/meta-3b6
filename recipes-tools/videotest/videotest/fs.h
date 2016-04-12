/** ***************************************************************************
 * @file fs.h
 * @brief
 **************************************************************************** */

#ifndef __FS_H
#define __FS_H

#include <stdint.h>

int fsDirGet(char* path, char* wildc, char** res, int size);
int fsFileLoad(char* name, uint8_t** buf);

#endif /* __FS_H */
