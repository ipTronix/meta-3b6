/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include "v4l2dev.h"

#define BUF_NUM 2

static int xioctl(int fh, int request, void *arg)
{
  int r;

  do {
    r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);
  return r;
}

int test(char* dev_i, char* dev_o);

/**
 */
static void usage(FILE *fp, int argc, char **argv)
{
  fprintf(fp,
            "Usage: %s [options]\n\n"
            "Version 1.3\n"
            "Options:\n"
            "-d | --device name   Video device output name [%s]\n"
            "-i | --input name    Video device input name [%s]\n"
            "-h | --help          Print this message\n"
            "-m | --mmap          Use memory mapped buffers [default]\n"
            "-r | --read          Use read() calls\n"
            "-u | --userp         Use application allocated buffers\n"
            "-o | --output        Outputs stream to stdout\n"
            "-f | --format        Force format to 640x480 YUYV\n"
            "-c | --count         Number of frames to grab [%i]\n"
            "",
            argv[0], "", 0);
}

static const char short_options[] = "d:i:hmruofc:";

static const struct option
long_options[] = {
  { "device", required_argument, NULL, 'd' },
  { "input",  required_argument, NULL, 'i' },
  { "help",   no_argument,       NULL, 'h' },
  { "mmap",   no_argument,       NULL, 'm' },
  { "read",   no_argument,       NULL, 'r' },
  { "userp",  no_argument,       NULL, 'u' },
  { "output", no_argument,       NULL, 'o' },
  { "format", no_argument,       NULL, 'f' },
  { "count",  required_argument, NULL, 'c' },
  { 0, 0, 0, 0 }
};

/**
 */
int main(int argc, char **argv)
{
  char *dev_name;
  char *dev_in;
  printf("capture "__DATE__" "__TIME__" pId %d\n", getpid());
  dev_name = "/dev/video4";
  dev_in = "/dev/video6";

  for (;;) {
    int idx;
    int c;

    c = getopt_long(argc, argv, short_options, long_options, &idx);
    if(c==-1){
      break;
    }
    switch (c) {
    case 0: /* getopt_long() flag */
            break;
    case 'd': dev_name = optarg; break;
    case 'i': dev_in   = optarg; break;
//    case 'm': io = IO_METHOD_MMAP; break;
//    case 'r': io = IO_METHOD_READ; break;
//    case 'u': io = IO_METHOD_USERPTR; break;
//    case 'o': out_buf++; break;
//    case 'f': force_format++; break;
/*    case 'c':
            errno = 0;
            frame_count = strtol(optarg, NULL, 0);
            if (errno)
              exit(EXIT_FAILURE);
            break;
*/
    case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);
    default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
    }
  }

  printf("call test(In %s, Out %s)\n", dev_in, dev_name);
  test(dev_in, dev_name);
  return 0;
}

/**
 */
int test(char* dev_i, char* dev_o)
{
  psDevHnd  pDev_i;
  psDevHnd  pDev_o;
  sDevBuf   pBuf[BUF_NUM];
  int       ret;
  int       i;
  struct v4l2_buffer  buf;
  struct v4l2_buffer  buf_i;
  struct v4l2_buffer  buf_o;
  enum v4l2_buf_type  type;

  pDev_i = devOpen(dev_i);
  if(pDev_i==NULL){
    return -1;
  }
  devShow(pDev_i);

  pDev_o = devOpen(dev_o);
  if(pDev_o==NULL){
    return -1;
  }
  devShow(pDev_o);

  printf("Request buffer for capture\n");
  devSetTypes(pDev_i, V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE);
//TODO
  ret = devBufReq(pDev_i, 
                  BUF_NUM, pBuf);//V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_MMAP,
  if(ret<=0){
    printf("Error Request buffer for capture\n");
    return -1;
  }

  printf("Request buffer for output\n");
  devSetTypes(pDev_o, V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT);
//TODO
  ret = devBufReq(pDev_o, 
                  BUF_NUM, pBuf);//V4L2_BUF_TYPE_VIDEO_OUTPUT, V4L2_MEMORY_USERPTR,
  if(ret<=0){
    printf("Error Request buffer for output\n");
    return -1;
  }


  // enqueue buffer
  for(i=0; i<BUF_NUM; ++i) {
    printf("%s - enqueue buffer %d\n", pDev_i->devname, i);
    memset(&buf, 0, sizeof(buf));
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index  = i;
    ret = xioctl(pDev_i->fd, VIDIOC_QBUF, &buf);
    if (ret == -1){
      printf("%s - ERROR VIDIOC_QBUF\n", pDev_i->devname);
      return -1;
    }

    printf("%s - enqueue buffer %d\n", pDev_o->devname, i);
    memset(&buf, 0, sizeof(buf));
    buf.type      = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory    = V4L2_MEMORY_USERPTR;
    buf.index     = i;
//    buf.m.userptr = (unsigned long)pBuf[i].addr;
//    buf.length    = pBuf[i].size;
    ret = xioctl(pDev_o->fd, VIDIOC_QBUF, &buf);
    if(ret == -1 ){
      printf("%s - ERROR VIDIOC_QBUF\n", pDev_o->devname);
      return -1;
    }
  }
/** ****** */
/*
{
#define ALTERAFW_CONTROL_SYNC_TYPE    (V4L2_CID_PRIVATE_BASE + 0)
#define ALTERAFW_CONTROL_DOUBLE_BUF   (V4L2_CID_PRIVATE_BASE + 1)

  struct v4l2_control ctrl;

  ctrl.id    = ALTERAFW_CONTROL_DOUBLE_BUF;
  ctrl.value = 1;
  ret = ioctl(pDev_i->fd, VIDIOC_G_CTRL, &ctrl);
  if(ret == -1){
    printf("%s - ERROR VIDIOC_G_CTRL %d %d %s\n", pDev_i->devname, ret, errno, strerror(errno));
    return -1;
  }
  printf("ALTERAFW_CONTROL_DOUBLE_BUF %d\n", ctrl.value);
  ctrl.value = 1;
  ret = ioctl(pDev_i->fd, VIDIOC_S_CTRL, &ctrl);
  if(ret == -1){
    printf("%s - ERROR VIDIOC_S_CTRL %d %d %s\n", pDev_i->devname, ret, errno, strerror(errno));
    return -1;
  }
}
*/
/** ****** */
  // start_capturing
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = xioctl(pDev_i->fd, VIDIOC_STREAMON, &type);
  if(ret == -1){
    printf("%s - ERROR VIDIOC_STREAMON\n", pDev_i->devname);
    return -1;
  }

  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ret = xioctl(pDev_o->fd, VIDIOC_STREAMON, &type);
  if(ret == -1){
    printf("%s - ERROR VIDIOC_STREAMON\n", pDev_o->devname);
    return -1;
  }

/*  for(;;){
    sleep(1);
  }*/
  for(;;){

    for(;;){
      fd_set          fds;
      struct timeval  tv;

      FD_ZERO(&fds);
      FD_SET(pDev_i->fd, &fds);

      /* Timeout. */
      tv.tv_sec  = 2;
      tv.tv_usec = 0;
      ret = select(pDev_i->fd + 1, &fds, NULL, NULL, &tv);
      if(ret==-1){
        if(errno==EINTR){
          continue;
        }
        fprintf(stderr, "select\n");
        return -1;
      }
      if(ret==0) {
        fprintf(stderr, "select timeout\n");
        return -1;
      }

      memset(&buf_i, 0, sizeof(buf_i));
      buf_i.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf_i.memory = V4L2_MEMORY_MMAP;
      ret = xioctl(pDev_i->fd, VIDIOC_DQBUF, &buf_i);
      if(ret){
        if(errno==EAGAIN){
          continue;
        }
        printf("%s - ERROR VIDIOC_DQBUF %d %d\n", pDev_i->devname, ret, errno);
        return -1;
      }
      break;
    }
/**/
    memcpy(&buf_o, &buf_i, sizeof(struct v4l2_buffer));
    buf_o.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf_o.memory = V4L2_MEMORY_USERPTR;

    for(;;){
      ret = xioctl(pDev_o->fd, VIDIOC_QBUF, &buf_o);
      if(ret){
        if(errno==EAGAIN){
          continue;
        }
        printf("%s - ERROR VIDIOC_QBUF %d %d %s\n", pDev_o->devname, ret, errno, strerror(errno));
        return -1;
      }
      break;
    }
    ret = xioctl(pDev_o->fd, VIDIOC_DQBUF, &buf_o);
    if(ret){
      printf("%s - ERROR VIDIOC_DQBUF\n", pDev_o->devname);
      return -1;
    }
/**/
    ret = xioctl(pDev_i->fd, VIDIOC_QBUF, &buf_i);
    if(ret){
      printf("%s - ERROR VIDIOC_QBUF %d %d %s\n", pDev_i->devname, ret, errno, strerror(errno));
      return -1;
    }
  }

  devClose(pDev_i);
  devClose(pDev_o);
}
