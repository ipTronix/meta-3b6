/** ***************************************************************************
 * @file fs.c
 * @brief
 **************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

/**
 */
int fsDirGet(char* path, char* wildc, char** res, int size)
{
  DIR            *d;
  struct dirent  *dir;
  int             wcl;
  int             num;

  num = 0;
  d = opendir(path);
  if(d){
    wcl = strlen(wildc);
    while( (dir = readdir(d)) != NULL ){
      if( memcmp(dir->d_name, wildc, wcl)==0 ){
        if(dir->d_type == DT_CHR){//      This is a character device.
          sprintf(res[num], "/dev/%s", dir->d_name);
          num++;
          if(num>size){
            return num;
          }
        }
      }
    }
    closedir(d);
  }
  return num;
}

/**
 */
int fsFileLoad(char* name, uint8_t** buf)
{
  int     fd;
  off_t   len;
  int     ret;

  fd = open(name, O_RDONLY);
  if(fd==-1){
    printf("ERROR:while open %s\n", name);
    return 0;
  }
  len = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  *buf = (char*)malloc(len);
  if(!*buf){
    close(fd);
    return 0;
  }
  ret = read(fd, *buf, len);
  if(ret!=len){
    close(fd);
    free(*buf);
    return 0;
  }
  close(fd);
  return len;
}
