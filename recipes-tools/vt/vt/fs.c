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
#include <sys/types.h>
#include <sys/stat.h>

#include "fs.h"

/**
 * @brief
 * @param path
 * @param wildc
 * @param res
 * @param size
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
          if(res){
            sprintf(res[num], "/dev/%s", dir->d_name);
          }
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
int fsFileStat(char* name, uint32_t type)
{
  struct stat   st;
  int           ret;

  ret = stat(name, &st);
  if(ret){
    return -1;
  }
  if((type & FS_TYPE_REG )&&(!S_ISREG (st.st_mode))){ return -1; }
  if((type & FS_TYPE_DIR )&&(!S_ISDIR (st.st_mode))){ return -1; }
  if((type & FS_TYPE_CHR )&&(!S_ISCHR (st.st_mode))){ return -1; }
  if((type & FS_TYPE_BLK )&&(!S_ISBLK (st.st_mode))){ return -1; }
  if((type & FS_TYPE_IFO )&&(!S_ISFIFO(st.st_mode))){ return -1; }
  if((type & FS_TYPE_LNK )&&(!S_ISLNK (st.st_mode))){ return -1; }
  if((type & FS_TYPE_SOCK)&&(!S_ISSOCK(st.st_mode))){ return -1; }

/*
  printf("%d %d\n", major(st.st_dev), minor(st.st_dev));
  if((st.st_mode&S_IFSOCK)==S_IFSOCK){printf("  socket\n");}
  if((st.st_mode&S_IFLNK )==S_IFLNK ){printf("  symbolic link\n");}
  if((st.st_mode&S_IFREG )==S_IFREG ){printf("  regular file\n");}
  if((st.st_mode&S_IFBLK )==S_IFBLK ){printf("  block device\n");}
  if((st.st_mode&S_IFDIR )==S_IFDIR ){printf("  directory\n");}
  if((st.st_mode&S_IFCHR )==S_IFCHR ){printf("  character device\n");}
  if((st.st_mode&S_IFIFO )==S_IFIFO ){printf("  FIFO\n");}
*/
/*
S_ISREG(m)  is it a regular file?
S_ISDIR(m)  directory?
S_ISCHR(m)  character device?
S_ISBLK(m)  block device?
S_ISFIFO(m) FIFO (named pipe)?
S_ISLNK(m)  symbolic link? (Not in POSIX.1-1996.)
S_ISSOCK(m) socket? (Not in POSIX.1-1996.)
*/
  return 0;
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
