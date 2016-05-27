/** ***************************************************************************
 * @file v4l2.c
 * @brief
 **************************************************************************** */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "v4l2.h"

//#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "CAP"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
//fprintf(stderr, "%s is no V4L2 device\n", pCap->name);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif

/**
 */
int v4l2InfoGet(char* dev, psV4l2Info pInf)
{
  int                     fd;
  int                     i;
  struct v4l2_capability  cap;
  struct v4l2_input       in;
  int                     ret;

  fd = open(dev, O_RDWR, 0);
  if(fd<0){
    printf("v4l2InfoGet:Unable to open '%s'\n", dev);
    return -1;
  }
  ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
  if(ret<0){
    if(errno==EINVAL){
      fprintf(stderr, "%s is not V4L2 device\n", dev);
      close(fd);
      return -1;
    } else {
      fprintf(stderr, "%s is not V4L device, unknow error\n", dev);
      close(fd);
      return -1;
    }
  }
  strcpy( pInf->device       , dev             );
  strcpy( pInf->driver       , cap.driver      );
  strcpy( pInf->card         , cap.card        );
  strcpy( pInf->bus_info     , cap.bus_info    );
          pInf->version      = cap.version      ;
          pInf->capabilities = cap.capabilities ;
          pInf->device_caps  = cap.device_caps  ;
          pInf->num_input    = 0;
  for(i=0; i<32; i++){
    in.index = i;
    ret = ioctl(fd, VIDIOC_ENUMINPUT, &in);
    if(ret){
      break;
    }
    strcpy( pInf->input[i].name         , in.name        );
            pInf->input[i].type         = in.type         ;
            pInf->input[i].status       = in.status       ;
            pInf->input[i].capabilities = in.capabilities ;
  }
  pInf->num_input = i;
  close(fd);
  return 0;
}

/**
 */
int v4l2InfoShow(psV4l2Info pInf)
{
  int                     fd;
  int                     i;
  struct v4l2_capability  cap;
  struct v4l2_input       in;
  int                     ret;

  printf("==============================================\n");
  printf("%s\n", pInf->device);
  printf("---------------------\n");
  printf( "driver   %s\n"
          "card     %s\n"
          "bus_info %s\n"
          "version  %08X\n",
          pInf->driver  ,
          pInf->card    ,
          pInf->bus_info,
          pInf->version );
  uint32_t c = pInf->capabilities;
  if( c & V4L2_CAP_VIDEO_CAPTURE       ) printf("Is a video capture device         \n");
  if( c & V4L2_CAP_VIDEO_OUTPUT        ) printf("Is a video output device          \n");
  if( c & V4L2_CAP_VIDEO_OVERLAY       ) printf("Can do video overlay              \n");
  if( c & V4L2_CAP_VBI_CAPTURE         ) printf("Is a raw VBI capture device       \n");
  if( c & V4L2_CAP_VBI_OUTPUT          ) printf("Is a raw VBI output device        \n");
  if( c & V4L2_CAP_SLICED_VBI_CAPTURE  ) printf("Is a sliced VBI capture device    \n");
  if( c & V4L2_CAP_SLICED_VBI_OUTPUT   ) printf("Is a sliced VBI output device     \n");
  if( c & V4L2_CAP_RDS_CAPTURE         ) printf("RDS data capture                  \n");
  if( c & V4L2_CAP_VIDEO_OUTPUT_OVERLAY) printf("Can do video output overlay       \n");
  if( c & V4L2_CAP_HW_FREQ_SEEK        ) printf("Can do hardware frequency seek    \n");
  if( c & V4L2_CAP_RDS_OUTPUT          ) printf("Is an RDS encoder                 \n");
  if( c & V4L2_CAP_VIDEO_CAPTURE_MPLANE) printf("VIDEO_CAPTURE_MPLANE              \n");
  if( c & V4L2_CAP_VIDEO_OUTPUT_MPLANE ) printf("VIDEO_OUTPUT_MPLANE               \n");
  if( c & V4L2_CAP_VIDEO_M2M_MPLANE    ) printf("VIDEO_M2M_MPLANE                  \n");
  if( c & V4L2_CAP_VIDEO_M2M           ) printf("VIDEO_M2M                         \n");
  if( c & V4L2_CAP_TUNER               ) printf("has a tuner                       \n");
  if( c & V4L2_CAP_AUDIO               ) printf("has audio support                 \n");
  if( c & V4L2_CAP_RADIO               ) printf("is a radio device                 \n");
  if( c & V4L2_CAP_MODULATOR           ) printf("has a modulator                   \n");
  if( c & V4L2_CAP_SDR_CAPTURE         ) printf("Is a SDR capture device           \n");
  if( c & V4L2_CAP_EXT_PIX_FORMAT      ) printf("Supports the extended pixel format\n");
  if( c & V4L2_CAP_READWRITE           ) printf("read/write systemcalls            \n");
  if( c & V4L2_CAP_ASYNCIO             ) printf("async I/O                         \n");
  if( c & V4L2_CAP_STREAMING           ) printf("streaming I/O ioctls              \n");
  if( c & V4L2_CAP_DEVICE_CAPS         ) printf("sets device capabilities field    \n");
  for(i=0; i<pInf->num_input; i++){
    printf( "Input index %d\n"
            "  name         %s\n"
            "  type         %08X\n"
            "  status       %08X\n"
            "  capabilities %08X\n",
            i                          ,
            pInf->input[i].name        ,
            pInf->input[i].type        ,
            pInf->input[i].status      ,
            pInf->input[i].capabilities);
  }
  return 0;
}
