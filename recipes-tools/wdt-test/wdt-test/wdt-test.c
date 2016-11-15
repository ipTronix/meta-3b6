/**
 * @file wdt-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include <pthread.h>
//#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/mount.h>

#define WDT_DEVICE  "/dev/watchdog"

/**
 */
typedef struct{
  int   fd;
}sWdtHnd, *psWdtHnd;

psWdtHnd  wdtOpen(void);
int       wdtSet(psWdtHnd pHnd, int timeout);
int       wdtTrigger(psWdtHnd pHnd);
int       wdtInfo(psWdtHnd pHnd);

/**
 * Main application
 */
int main(int argc, char** argv)
{
  psWdtHnd  pWdt;
  int       i;
  int       ret;

  pWdt = wdtOpen();
  if(!pWdt){
    printf("ERROR opening watchdog device\n");
    return -1;
  }

  ret = wdtInfo(pWdt);
  if(ret){
    printf("ERROR getting watchdog informations\n");
    return -1;
  }

  ret = wdtSet(pWdt, 2);
  if(ret){
    printf("ERROR setting watchdog\n");
    return -1;
  }

  i = 0;
  for(;;){
    if(i<3){
      printf("trigger %d\n", i);
      ret = wdtTrigger(pWdt);
      if(ret){
        printf("ERROR triggering watchdog\n");
        break;
      }
    }else{
      printf("wait %d\n", i);
    }
    i++;
    sleep(1);
  }

  return 0;
}


/**
 */
psWdtHnd wdtOpen(void)
{
  psWdtHnd pHnd;

  pHnd = (psWdtHnd)malloc(sizeof(sWdtHnd));
  if(!pHnd){
    return NULL;
  }
  memset(pHnd, 0, sizeof(sWdtHnd));

  pHnd->fd = open(WDT_DEVICE, O_WRONLY);
  if(pHnd->fd<0){
    printf("Error opening watchdog device %s\n", WDT_DEVICE);
    free(pHnd);
    return NULL;
  }
  // TODO get info & C.
  return pHnd;
}

/**
 */
int wdtSet(psWdtHnd pHnd, int timeout)
{
  int ret;
  int t;

  ret = ioctl(pHnd->fd, WDIOC_SETTIMEOUT, &timeout);
  if(ret){
    return -1;
  }
  ret = ioctl(pHnd->fd, WDIOC_GETTIMEOUT, &t);
  if(t!=timeout){
    printf("warning: can't set watchdog timeout value\n");
    return -2;
  }
  printf("watchdog timeout set at %d seconds\n", timeout);
  return 0;
}

/**
 */
int wdtTrigger(psWdtHnd pHnd)
{
  int ret;
  ret = ioctl(pHnd->fd, WDIOC_KEEPALIVE, 0);
  if(ret){
    return -1;
  }
  return 0;
}

/**
 */
int wdtInfo(psWdtHnd pHnd)
{
  struct watchdog_info  wdti;
  uint32_t              tmp;
  int                   ret;

  ret = ioctl(pHnd->fd, WDIOC_GETSUPPORT, &wdti);
  if(ret){
    return -1;
  }
  printf( "Support\n"
          "  options          %08X\n"  /* Options the card/driver supports */
          "  firmware_version %08X\n"  /* Firmware version of the card */
          "  identity         %s\n",   /* Identity of the board */
          wdti.options,
          wdti.firmware_version,
          wdti.identity);

  ret = ioctl(pHnd->fd, WDIOC_GETSTATUS, &tmp);
  if(ret){
    return -1;
  }
  printf( "Status %08X\n", tmp);

  ret = ioctl(pHnd->fd, WDIOC_GETBOOTSTATUS, &tmp);
  if(ret){
    return -1;
  }
  printf( "Boot status %08X\n", tmp);

  ret = ioctl(pHnd->fd, WDIOC_GETPRETIMEOUT, &tmp);
  if(ret){
    return -1;
  }
  printf( "Pre timeout %08X\n", tmp);
  return 0;
}
/*
  case WDIOC_SETPRETIMEOUT:
    if (get_user(new_value, p))
      return -EFAULT;
    if ((new_value < 0) || (new_value >= imx2_wdt.timeout))
      return -EINVAL;
    imx2_wdt_set_pretimeout(new_value);
    imx2_wdt.pretimeout = new_value;
*/

/*

 https://www.kernel.org/doc/Documentation/watchdog/watchdog-api.txt

All watchdog drivers are required return more information about the system,
some do temperature, fan and power level monitoring, some can tell you
the reason for the last reboot of the system.  The GETSUPPORT ioctl is
available to ask what the device can do:

  struct watchdog_info ident;
  ioctl(fd, WDIOC_GETSUPPORT, &ident);

the fields returned in the ident struct are:

        identity    a string identifying the watchdog driver
  firmware_version  the firmware version of the card if available
  options     a flags describing what the device supports

the options field can have the following bits set, and describes what
kind of information that the GET_STATUS and GET_BOOT_STATUS ioctls can
return.   [FIXME -- Is this correct?]

  WDIOF_OVERHEAT    Reset due to CPU overheat

The machine was last rebooted by the watchdog because the thermal limit was
exceeded

  WDIOF_FANFAULT    Fan failed

A system fan monitored by the watchdog card has failed

  WDIOF_EXTERN1   External relay 1

External monitoring relay/source 1 was triggered. Controllers intended for
real world applications include external monitoring pins that will trigger
a reset.

  WDIOF_EXTERN2   External relay 2

External monitoring relay/source 2 was triggered

  WDIOF_POWERUNDER  Power bad/power fault

The machine is showing an undervoltage status

  WDIOF_CARDRESET   Card previously reset the CPU

The last reboot was caused by the watchdog card

  WDIOF_POWEROVER   Power over voltage

The machine is showing an overvoltage status. Note that if one level is
under and one over both bits will be set - this may seem odd but makes
sense.

  WDIOF_KEEPALIVEPING Keep alive ping reply

The watchdog saw a keepalive ping since it was last queried.

  WDIOF_SETTIMEOUT  Can set/get the timeout

The watchdog can do pretimeouts.

  WDIOF_PRETIMEOUT  Pretimeout (in seconds), get/set









For those drivers that return any bits set in the option field, the
GETSTATUS and GETBOOTSTATUS ioctls can be used to ask for the current
status, and the status at the last reboot, respectively.

    int flags;
    ioctl(fd, WDIOC_GETSTATUS, &flags);

    or

    ioctl(fd, WDIOC_GETBOOTSTATUS, &flags);

Note that not all devices support these two calls, and some only
support the GETBOOTSTATUS call.

Some drivers can measure the temperature using the GETTEMP ioctl.  The
returned value is the temperature in degrees fahrenheit.

    int temperature;
    ioctl(fd, WDIOC_GETTEMP, &temperature);

Finally the SETOPTIONS ioctl can be used to control some aspects of
the cards operation.

    int options = 0;
    ioctl(fd, WDIOC_SETOPTIONS, &options);

The following options are available:

  WDIOS_DISABLECARD Turn off the watchdog timer
  WDIOS_ENABLECARD  Turn on the watchdog timer
  WDIOS_TEMPPANIC   Kernel panic on temperature trip



 */