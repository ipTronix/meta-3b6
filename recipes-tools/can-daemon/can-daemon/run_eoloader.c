// file:      can_daemon/run_eoloader.c
// project:   vt3
// copyright: TECHNOLEADER S.R.L. all rights reserved

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>


#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#include <sys/ioctl.h>
#include <linux/fb.h>
#endif

#include "run_eoloader.h"


// this timeout defines the time [ms] to wait before checking for usb devices
#define USBDEV_PROBE_TOUT	(40000)

// taken from watchdog.h
#define	WATCHDOG_IOCTL_BASE	'W'
#ifndef WIN32
#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE,   5, int)
#else
#define	WDIOC_KEEPALIVE		(10)
int ioctl(int d, unsigned long request, unsigned int n);
#endif


// this flag is set when the procedure is run, in order not to execute it more than once
static int eoloader_completed   = 0;
// starting time
static struct timespec run_eoloader_start;
// starting time flag
static unsigned int system_has_booted = 0;
// watchdog file descriptor
static int wdg_fd = -1;
// flag to terminate watchdog trigger thread
static int close_wd_thread_flag = 0;
// watchdog thread
static pthread_t wd_thread;
// flag to terminate splash thread
static int close_splash_thread_flag = 0;
// splash thread
static pthread_t splash_thread;

/* convert timespec to milliseconds */
static long long timespec_to_ms(struct timespec t)
{
    return t.tv_sec * 1000LL + t.tv_nsec / 1000000LL;
}

// These functions must be used ONLY for test purposes. 
// This function resets the flag eoloader_completed
void reset_eoloader_completed()
{
    eoloader_completed =0;
}
// This function sets starting time (to test schedule_eoloader function)
void set_starting_time()
{
    (void) clock_gettime(CLOCK_MONOTONIC, &run_eoloader_start);
}

// This function checks if the usb probe time has elapsed in order to proceed with the usb mounting.
// If the time has expired the mount is tried only once and is never retried.
unsigned int schedule_eoloader()
{
	// 10 seconds check
	struct timespec run_eoloader_time;
	(void) clock_gettime(CLOCK_MONOTONIC, &run_eoloader_time);
	if ((timespec_to_ms(run_eoloader_time) - timespec_to_ms(run_eoloader_start) < USBDEV_PROBE_TOUT) && (eoloader_completed == 0))
		return 0;

	return 1;
}


static int call_system_checking_error(char const * command)
{
	printf("command: \"%s\"\n", command);
	int const err = system(command);
	if ( err )
		printf("error: command returned %d\n", err);
	return err;
}



// This function tries to mount the usb device to /mnt/floppy
// return 1 if success 0 if failure
unsigned int try_usb_mount(const char * device_dirname)
{
	// formatting mounting command
	char mount_cmd[64];
	snprintf(mount_cmd, sizeof mount_cmd,
			"mount -t vfat %s /mnt/floppy",
			device_dirname);

	// execute mount command
	(void) call_system_checking_error(mount_cmd);

	// check mount result - sometimes the error code is not reliable
	FILE * f = popen("mount | grep /mnt/floppy | wc -l", "r");
	if ( f == NULL )
		return 0;
	char line[32];
	if ( fgets(line, sizeof line, f) == NULL )
	{
		pclose(f);
		return 0;
	}
	pclose(f);
	int const nmount = atoi(line);
	return (nmount == 1);
}






/* finds a file matching the glob pattern. return 1 if success */
static int find_glob_filename(char const * globpattern, char * filename, size_t buflen)
{
	strcpy(filename, "");

	char cmd[256];
	snprintf(cmd, sizeof cmd, "ls %s", globpattern);
    FILE * f = popen(cmd, "r");
    if ( f == NULL )
    {
    	return 0;
    }

    if ( fgets(filename, buflen - 1, f) == 0 )
    {
    }
    else
    {
    	unsigned len = strlen(filename);
    	if ( filename[len - 1] == '\n' )
		{
    		filename[len - 1] = '\0';
		}
    }
    pclose(f);

    return access(filename, R_OK) == 0;
}





static char eoloader_application_zip[256];
//static char eoloader_configuration_zipromfs[256];




// This function looks for zip files in the usb device
unsigned int check_for_zip_files()
{
	if ( ! find_glob_filename("/mnt/floppy/frontendTera-TeraEOloader-*.zip", eoloader_application_zip, sizeof eoloader_application_zip) )
		return 0;
    printf("eoloader_application_zipfile = \"%s\"\n", eoloader_application_zip);

/*

 	if ( ! find_glob_filename("/mnt/floppy/eoloader_tera_cfg_*.zipromfs", eoloader_configuration_zipromfs, sizeof eoloader_configuration_zipromfs) )
		return 0;
    printf("eoloader_configuration_zipfile = \"%s\"\n", eoloader_configuration_zipromfs);
*/

    // the files are there, returning 1
	return 1;
}




static int unzip_eoloader_application(char const * zipfilename, char const * destination)
{
	time_t start = time(0);
    char cmd[256];
    snprintf(cmd, sizeof cmd,
    		"unzip %s -d %s",
    		zipfilename,
    		destination);
    printf("%s\n", cmd);
    int const err = call_system_checking_error(cmd);
    printf("unzip took %ld seconds\n", time(0) - start);
	if (err != 0)
	{
		printf("unzip failed: %d\n", err);
		return err;
	}
	printf("unzip returned with success\n");
	return 0;
}


// This function inflates the application
unsigned int prepare_application(void)
{
	// create mount directories
	call_system_checking_error("mkdir /mnt/floppy/eoloader_tera_bin    > /dev/null");
	// system("mkdir /mnt/floppy/eoloader_tera_cfg    > /dev/null");
	call_system_checking_error("rm -rf /mnt/floppy/eoloader_tera_bin/* > /dev/null");
    // system("rm -rf /mnt/floppy/eoloader_tera_cfg/* > /dev/null");

    // stop vt3_app
	call_system_checking_error("killall vt3_app");
    sleep(1);
    call_system_checking_error("killall -9 vt3_app");
    sleep(1);

    // inflate
	if (unzip_eoloader_application(eoloader_application_zip, "/mnt/floppy/eoloader_tera_bin") != 0)
		return 0;

    // mount configuration file
	// if (mount_loopback_romfs(eoloader_configuration_zipromfs, "/mnt/floppy/eoloader_tera_cfg") != 0)
	// 	return 0;
	
	return 1;
}

// This function launches the eoloader application
void launch_eoloader()
{
	// check if host device is IMX31 or IMX35
	int const err = call_system_checking_error("is_qws_mouse_defined=`env | grep QWS_MOUSE_PROTO | wc -l`; exit $is_qws_mouse_defined");
    char const * qws_mouse_proto_variable = (err != 0) ? "" :
		"export QWS_MOUSE_PROTO=tslib:/dev/input/tslib0;export QT_QWS_FONTDIR=/mnt/floppy/eoloader_tera_bin/fonts;export QWS_KEYBOARD=linuxinput:/dev/input/event0;";

	char launch_command[256];
    snprintf(launch_command, sizeof launch_command,
    		"cd /mnt/floppy/eoloader_tera_bin;export LD_LIBRARY_PATH=/mnt/floppy/eoloader_tera_bin;%s./vt3_app", qws_mouse_proto_variable);
    printf ("**************** ATTENTION PLEASE ****************\n");
    printf("%s\n", launch_command);
    printf ("**************** **************** ****************\n");
    call_system_checking_error("env");
    call_system_checking_error(launch_command);
}



// unmount the usb flash
void unmount_usb_device()
{
	call_system_checking_error("rm -Rf /mnt/floppy/eoloader_tera_bin");
	call_system_checking_error("sync");
	call_system_checking_error("umount /mnt/floppy");
	call_system_checking_error("sync");
}


// screen resolution width and height
// set by reinitialize_framebuffer
// used by cycle_splash_screen
static unsigned resolution_width, resolution_height;



// vt3_app leaves the framebuffer in a state where "cat xxx > /dev/fb0" does not work
// so we need to reinitialize...
static void reinitialize_framebuffer(void)
{
	int framebuffer_device;
	int line_size, buffer_size;
	int *screen_memory;
	struct fb_var_screeninfo var_info;

	framebuffer_device = open ("/dev/fb0", O_RDWR);
	ioctl (framebuffer_device, FBIOGET_VSCREENINFO, &var_info);

	line_size = var_info.xres * var_info.bits_per_pixel / 8;
	buffer_size = line_size * var_info.yres;

	resolution_width = var_info.xres;
	resolution_height = var_info.yres;

	var_info.xoffset = 0;
	var_info.yoffset = 0;
	ioctl (framebuffer_device, FBIOPAN_DISPLAY, &var_info);

	screen_memory = (int *) mmap (NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, framebuffer_device, 0);
	memset(screen_memory, 0x55, buffer_size);
	close(framebuffer_device);
}




// rotate the splash image on the screen
static void cycle_splash_screen(void)
{
	// rotate splash index
	static unsigned splash_index = 1;
	if ( splash_index == 3 )
		splash_index = 1;
	else
		splash_index ++;

	// uncompress splash first time
	char cmd[256];
	char splash_filename[64];
	snprintf(splash_filename, sizeof splash_filename, "/tmp/splash/eoloader_splash_%ux%u_%u.fb",
			resolution_width, resolution_height, splash_index);
	if ( access(splash_filename, R_OK) != 0 )
	{
		snprintf(cmd, sizeof cmd,
				"unzip -n %s splash/eoloader_splash_%ux%u_%u.fb -d /tmp",
				eoloader_application_zip,
				resolution_width, resolution_height,
				splash_index);
		printf("extract splash from zip with command:\n%s\n", cmd);
		system(cmd);
	}

	// show splash
	snprintf(cmd, sizeof cmd, "cat %s > /dev/fb0", splash_filename);
	call_system_checking_error(cmd);
}



// this function shows the splash
static void * show_splash(void * param)
{
	printf("starting thread: show_splash()\n");

	const unsigned CYCLE_TIME = 3; // seconds for splash animation

	// remove log file
	call_system_checking_error("rm /mnt/floppy/eoloader.log");

    // reinitialize the framebuffer, twice to remove artifacts
	printf("reinitialize_framebuffer\n");
	reinitialize_framebuffer();

	// thread loop
    do
    {
    	// rotate the splash image on the screen
    	cycle_splash_screen();

        // animation
        sleep(CYCLE_TIME);

        // when the eoloader log file is created, stop splash
        if ( access("/mnt/floppy/eoloader.log", R_OK) == 0 )
        {
        	break;
        }

    } while ( ! close_splash_thread_flag );

	// remove temporary splash files
	call_system_checking_error("rm -rf /tmp/splash/");

	printf("ending thread: show_splash()\n");
    return NULL;
}

// this function launches the thread which triggers the hardware watchdog
unsigned int launch_splash_thread(void)
{
    int rc;

    rc = pthread_create(&splash_thread, NULL, show_splash, NULL);
    if (rc)
    {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        return 0;
    }
    return 1;
}

// this function launches the thread which triggers the hardware watchdog
void close_splash_thread()
{
	printf("enter close_splash_thread\n");
    close_splash_thread_flag = 1;
    pthread_join(splash_thread, NULL);
	printf("exit close_splash_thread\n");
}




// this function triggers HW watchdog
static void * trigger_hw_wdog(void * param)
{
	printf("starting thread: trigger_hw_wdog()\n");

	const unsigned CYCLE_TIME = 10; // seconds for watchdog trigger

	// thread loop
    while ( ! close_wd_thread_flag )
    {
    	// trigger watchdog
        if (wdg_fd >= 0)
        	ioctl(wdg_fd, WDIOC_KEEPALIVE, 0);

        // period
        sleep(CYCLE_TIME);
    }

	printf("ending thread: trigger_hw_wdog()\n");
    return NULL;
}

// this function launches the thread which triggers the hardware watchdog
unsigned int launch_wd_trigger_thread(void)
{
    int rc;

    rc = pthread_create(&wd_thread, NULL, trigger_hw_wdog, NULL);
    if (rc)
    {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        return 0;
    }
    return 1;

}

// this function launches the thread which triggers the hardware watchdog
void close_wd_trigger_thread()
{
	printf("close_wd_trigger_thread\n");
    close_wd_thread_flag = 1;
    pthread_join(wd_thread, NULL);
}


/***********************************************************************
*
*	This function looks for EOloader scripts in USB pen and executes them
*/
void run_eoloader(int wdog_fd)
{
	// set of start time
	if (0 == system_has_booted)
	{
        (void) clock_gettime(CLOCK_MONOTONIC, &run_eoloader_start);
		system_has_booted = 1;
	}

    // waiting for usb dev driver to be probed
    if ( eoloader_completed == 1 || 0 == schedule_eoloader() )
		return;

	// trying mounts
    // sometimes for unknown reasons the system connects the usb flash drive to sdb1 instead of sda1
    // so we try both
	if (0 == try_usb_mount("/dev/sda1"))
	{
		if (0 == try_usb_mount("/dev/sdb1"))
		{
			eoloader_completed = 1;
			return;
		}
	}

	// looking for zipromfs files
	if ( 0 == check_for_zip_files())
	{
		eoloader_completed = 1;
		return;
	}

    // launching watchdog trigger and splash thread
    wdg_fd = wdog_fd;
    launch_wd_trigger_thread();
    launch_splash_thread();

	// inflate and prepare application
	if ( 0 == prepare_application() )
	{
        eoloader_completed = 1;
		return;
	}

	// run eoloader
	launch_eoloader();

	// shut down splash cycle thread
    close_splash_thread();

	// shut down watchdog trigger thread
    close_wd_trigger_thread();

   	// unmount devices
	unmount_usb_device();
	
	// eoloader procedure terminated
	eoloader_completed = 1;
}


// end of file

