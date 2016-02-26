/**
 * @file wdt-test.c
 */
#include <stdio.h>
#include <stdlib.h>
//#include <pthread.h>
//#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
//#include <math.h>
#include <sys/ioctl.h>
// TODO #include <watchdog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/mount.h>

/* internal watchdog */
//@todo ripristinare
#define WDG
int  wdg_fd = 0; // file descriptor for watchdog
#ifdef WDG
/* taken from watchdog.h */
#define	WATCHDOG_IOCTL_BASE	'W'
#define	WDIOC_KEEPALIVE     _IOR (WATCHDOG_IOCTL_BASE,  5, int)
#define	WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE,  6, int)
#define	WDIOC_GETTIMEOUT    _IOR (WATCHDOG_IOCTL_BASE,  7, int)
#define	WDIOC_GETTIMELEFT   _IOR (WATCHDOG_IOCTL_BASE, 10, int)
//--GC
#ifdef BOARD_IMX356_SMALL
#define WD_DEV	"/dev/ext_watchdog"
#else
#define WD_DEV	"/dev/watchdog"
#endif // BOARD_IMX356_SMALL
#define WD_DEV_OLD	"/dev/misc/watchdog"
#define WD_TIMEOUT_VALUE 10
#endif


/**
 *
 * init_watchdog
 *
 * setup the freescale internal watchdog
 *
 */
static void init_watchdog(void)
{
#ifdef WDG
	int timeout;

	/* open wdg driver */
	//--GC: used for CRAMFS
	wdg_fd = open(WD_DEV, O_WRONLY);
	if (wdg_fd < 0){
		// try with the old style device name
		wdg_fd = open(WD_DEV_OLD, O_WRONLY);
	}
	if (wdg_fd < 0){
		printf("Error opening watchdog devices %s or %s\n", WD_DEV, WD_DEV_OLD);
		return;
	}

	/* set timeout: one minute */
	timeout = WD_TIMEOUT_VALUE;
	ioctl(wdg_fd, WDIOC_SETTIMEOUT, &timeout);

	/* read back timeout */
	timeout = 0;
	ioctl(wdg_fd, WDIOC_GETTIMEOUT, &timeout);
	if (timeout != WD_TIMEOUT_VALUE){
		printf("warning: can't set watchdog timeout value\n");
	}else{
		printf("watchdog timeout set at %d seconds\n", timeout);
	}
#endif
}

/**
 *
 * trigger_watchdog
 *
 * trigger the freescale internal watchdog
 *
 */
static void trigger_watchdog(void)
{
#ifdef WDG
	/* trigger watchdog all the time */
	if (wdg_fd < 0) {
		return;
	}
	ioctl(wdg_fd, WDIOC_KEEPALIVE, 0);
#endif
}

/**
 *
 * Main application
 *
 */
int main(int argc, char *argv[])
{
  printf("Init watchdog\n");
  init_watchdog();

  printf("Run loop\n");
  while(1){
    //trigger_watchdog();
    sleep(1);
  }
  return 0;
}
