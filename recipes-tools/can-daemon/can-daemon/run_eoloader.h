// file:      can_daemon/run_eoloader.h
// project:   vt3
// copyright: TECHNOLEADER S.R.L. all rights reserved

// The following functions must be used ONLY for test purposes.
#ifndef RUN_EOL_H
#define RUN_EOL_H

// This resets the flag eoloader_completed
// If the time has expired the mount is tried only once and is never retried.
void reset_eoloader_completed();
// This function sets starting time (to test schedule_eoloader function)
void set_starting_time();

// This function checks if the usb probe time has elapsed in order to proceed with the usb mounting.
// If the time has expired the mount is tried only once and is never retried.
unsigned int schedule_eoloader();

// This function tries to mount the usb device to /mnt/floppy
unsigned int try_usb_mount(const char * device_dirname);

// This function looks for zip files in the usb device
unsigned int check_for_zip_files();

// This function unzips zip files in the usb device
unsigned int unzip_eoloader_files();

// This function launches the eoloader application
void launch_eoloader();

// This function shuts the HW watchdog trigger down
void unmount_usb_device();

// This function looks for EOloader scripts in USB pen and executes them
void run_eoloader(int wdog_fd);

#endif  /* RUN_EOL_H */
