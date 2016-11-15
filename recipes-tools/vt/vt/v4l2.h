/** ***************************************************************************
 * @file v4l2.h
 * @brief
 **************************************************************************** */
#ifndef __V4L2_H
#define __V4L2_H

typedef struct {
  char        name[32];
  uint32_t    type;
  uint32_t    status;
  uint32_t    capabilities;
}sV4l2Input, *psV4l2Input;

typedef struct {
  char        device[32];
  char        driver[16];
  char        card[32];
  char        bus_info[32];
  uint32_t    version;
  uint32_t    capabilities;
  uint32_t    device_caps;
  uint8_t     num_input;
  sV4l2Input  input[32];
}sV4l2Info, *psV4l2Info;

int v4l2InfoGet(char* dev, psV4l2Info pInf);
int v4l2InfoShow(psV4l2Info pInf);

int v4l2Brightness(int fd, int32_t value);
int v4l2Hue       (int fd, int32_t value);
int v4l2Contrast  (int fd, int32_t value);
int v4l2Saturation(int fd, int32_t value);
int v4l2Sharpness (int fd, int32_t value);

#endif  /* __V4L2_H */
