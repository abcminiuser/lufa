/* Modified for the LUFA HID Bootloader by Dean Camera
 *           http://www.lufa-lib.org
 *
 *   THIS MODIFIED VERSION IS UNSUPPORTED BY PJRC.
 */

/* Teensy Loader, Command Line Interface
 * Program and Reboot Teensy Board with HalfKay Bootloader
 * http://www.pjrc.com/teensy/loader_cli.html
 * Copyright 2008-2016, PJRC.COM, LLC
 *
 * You may redistribute this program and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

/* Want to incorporate this code into a proprietary application??
 * Just email paul@pjrc.com to ask.  Usually it's not a problem,
 * but you do need to ask to use this code in any way other than
 * those permitted by the GNU General Public License, version 3  */

/* For non-root permissions on ubuntu or similar udev-based linux
 * http://www.pjrc.com/teensy/49-teensy.rules
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

void usage(const char *err)
{
	if(err != NULL) fprintf(stderr, "%s\n\n", err);
	fprintf(stderr,
		"Usage: hid_bootloader_cli --mcu=<MCU> [-w] [-h] [-n] [-b] [-v] <file.hex>\n"
		"\t-w : Wait for device to appear\n"
		"\t-r : Use hard reboot if device not online\n"
		"\t-s : Use soft reboot if device not online (Teensy 3.x & 4.x)\n"
		"\t-n : No reboot after programming\n"
		"\t-b : Boot only, do not program\n"
		"\t-v : Verbose output\n"
		"\nUse `hid_bootloader_cli --list-mcus` to list supported MCUs.\n"
		"\nFor support and more information, please visit:\n"
		"\thttp://www.lufa-lib.org\n"

		"\nBased on the TeensyHID command line programmer software:\n"
		"\thttp://www.pjrc.com/teensy/loader_cli.html\n");
	exit(1);
}

// USB Access Functions
int teensy_open(void);
int teensy_write(void *buf, int len, double timeout);
void teensy_close(void);
int hard_reboot(void);
int soft_reboot(void);

// Intel Hex File Functions
int read_intel_hex(const char *filename);
int ihex_bytes_within_range(int begin, int end);
void ihex_get_data(int addr, int len, unsigned char *bytes);
int memory_is_blank(int addr, int block_size);

// Misc stuff
int printf_verbose(const char *format, ...);
void delay(double seconds);
void die(const char *str, ...);
void parse_options(int argc, char **argv);
void boot(unsigned char *buf, int write_size);

// options (from user via command line args)
int wait_for_device_to_appear = 0;
int hard_reboot_device = 0;
int soft_reboot_device = 0;
int reboot_after_programming = 1;
int verbose = 0;
int boot_only = 0;
int code_size = 0, block_size = 0;
const char *filename=NULL;


/****************************************************************/
/*                                                              */
/*                       Main Program                           */
/*                                                              */
/****************************************************************/

int main(int argc, char **argv)
{
	unsigned char buf[2048];
	int num, addr, r, write_size;

	int first_block=1, waited=0;

	// parse command line arguments
	parse_options(argc, argv);
	if (!filename && !boot_only) {
		usage("Filename must be specified");
	}
	if (!code_size) {
		usage("MCU type must be specified");
	}
	printf_verbose("Teensy Loader, Command Line, Version 2.2\n");

	if (block_size == 512 || block_size == 1024) {
		write_size = block_size + 64;
	} else {
		write_size = block_size + 2;
	};

	if (!boot_only) {
		// read the intel hex file
		// this is done first so any error is reported before using USB
		num = read_intel_hex(filename);
		if (num < 0) die("error reading intel hex file \"%s\"", filename);
		printf_verbose("Read \"%s\": %d bytes, %.1f%% usage\n",
			filename, num, (double)num / (double)code_size * 100.0);
	}

	// open the USB device
	while (1) {
		if (teensy_open()) break;
		if (hard_reboot_device) {
			if (!hard_reboot()) die("Unable to find rebootor\n");
			printf_verbose("Hard Reboot performed\n");
			hard_reboot_device = 0; // only hard reboot once
			wait_for_device_to_appear = 1;
		}
		if (soft_reboot_device) {
			if (soft_reboot()) {
				printf_verbose("Soft reboot performed\n");
			}
			soft_reboot_device = 0;
			wait_for_device_to_appear = 1;
		}
		if (!wait_for_device_to_appear) die("Unable to open device (hint: try -w option)\n");
		if (!waited) {
			printf_verbose("Waiting for Teensy device...\n");
			printf_verbose(" (hint: press the reset button)\n");
			waited = 1;
		}
		delay(0.25);
	}
	printf_verbose("Found HalfKay Bootloader\n");

	if (boot_only) {
		boot(buf, write_size);
		teensy_close();
		return 0;
	}

	// if we waited for the device, read the hex file again
	// perhaps it changed while we were waiting?
	if (waited) {
		num = read_intel_hex(filename);
		if (num < 0) die("error reading intel hex file \"%s\"", filename);
		printf_verbose("Read \"%s\": %d bytes, %.1f%% usage\n",
		 	filename, num, (double)num / (double)code_size * 100.0);
	}

	// program the data
	printf_verbose("Programming");
	fflush(stdout);
	for (addr = 0; addr < code_size; addr += block_size) {
		if (!first_block && !ihex_bytes_within_range(addr, addr + block_size - 1)) {
			// don't waste time on blocks that are unused,
			// but always do the first one to erase the chip
			continue;
		}
		if (!first_block && memory_is_blank(addr, block_size)) continue;
		printf_verbose(".");
		if (block_size <= 256 && code_size < 0x10000) {
			buf[0] = addr & 255;
			buf[1] = (addr >> 8) & 255;
			ihex_get_data(addr, block_size, buf + 2);
			write_size = block_size + 2;
		} else if (block_size == 256) {
			buf[0] = (addr >> 8) & 255;
			buf[1] = (addr >> 16) & 255;
			ihex_get_data(addr, block_size, buf + 2);
			write_size = block_size + 2;
		} else if (block_size == 512 || block_size == 1024) {
			buf[0] = addr & 255;
			buf[1] = (addr >> 8) & 255;
			buf[2] = (addr >> 16) & 255;
			memset(buf + 3, 0, 61);
			ihex_get_data(addr, block_size, buf + 64);
			write_size = block_size + 64;
		} else {
			die("Unknown code/block size\n");
		}
		r = teensy_write(buf, write_size, first_block ? 5.0 : 0.5);
		if (!r) die("error writing to Teensy\n");
		first_block = 0;
	}
	printf_verbose("\n");

	// reboot to the user's new code
	if (reboot_after_programming) {
		boot(buf, write_size);
	}
	teensy_close();
	return 0;
}




/****************************************************************/
/*                                                              */
/*             USB Access - libusb (Linux & FreeBSD)            */
/*                                                              */
/*  Uses libusb v0.1. To install:                               */
/*  - [debian, ubuntu, mint] apt install libusb-dev             */
/*  - [redhat, centos]       yum install libusb-devel           */
/*  - [fedora]               dnf install libusb-devel           */
/*  - [arch linux]           pacman -S libusb-compat            */
/*  - [gentoo]               emerge dev-libs/libusb-compat      */
/*                                                              */
/*  - [freebsd]              seems to be preinstalled           */
/****************************************************************/

#if defined(USE_LIBUSB)

// http://libusb.sourceforge.net/doc/index.html
#include <usb.h>

usb_dev_handle * open_usb_device(int vid, int pid)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *h;
	char buf[128];
	int r;

	usb_init();
	usb_find_busses();
	usb_find_devices();
	//printf_verbose("\nSearching for USB device:\n");
	for (bus = usb_get_busses(); bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			//printf_verbose("bus \"%s\", device \"%s\" vid=%04X, pid=%04X\n",
			//	bus->dirname, dev->filename,
			//	dev->descriptor.idVendor,
			//	dev->descriptor.idProduct
			//);
			if (dev->descriptor.idVendor != vid) continue;
			if (dev->descriptor.idProduct != pid) continue;
			h = usb_open(dev);
			if (!h) {
				printf_verbose("Found device but unable to open\n");
				continue;
			}
			#ifdef LIBUSB_HAS_GET_DRIVER_NP
			r = usb_get_driver_np(h, 0, buf, sizeof(buf));
			if (r >= 0) {
				r = usb_detach_kernel_driver_np(h, 0);
				if (r < 0) {
					usb_close(h);
					printf_verbose("Device is in use by \"%s\" driver\n", buf);
					continue;
				}
			}
			#endif
			// Mac OS-X - removing this call to usb_claim_interface() might allow
			// this to work, even though it is a clear misuse of the libusb API.
			// normally Apple's IOKit should be used on Mac OS-X
			#if !defined(MACOSX)
			r = usb_claim_interface(h, 0);
			if (r < 0) {
				usb_close(h);
				printf_verbose("Unable to claim interface, check USB permissions\n");
				continue;
			}
			#endif

			return h;
		}
	}
	return NULL;
}

static usb_dev_handle *libusb_teensy_handle = NULL;

int teensy_open(void)
{
	teensy_close();
	libusb_teensy_handle = open_usb_device(0x16C0, 0x0478);

	if (!libusb_teensy_handle)
		libusb_teensy_handle = open_usb_device(0x03eb, 0x2067);

	if (libusb_teensy_handle) return 1;
	return 0;
}

int teensy_write(void *buf, int len, double timeout)
{
	int r;

	if (!libusb_teensy_handle) return 0;
	while (timeout > 0) {
		r = usb_control_msg(libusb_teensy_handle, 0x21, 9, 0x0200, 0,
			(char *)buf, len, (int)(timeout * 1000.0));
		if (r >= 0) return 1;
		//printf("teensy_write, r=%d\n", r);
		usleep(10000);
		timeout -= 0.01;  // TODO: subtract actual elapsed time
	}
	return 0;
}

void teensy_close(void)
{
	if (!libusb_teensy_handle) return;
	usb_release_interface(libusb_teensy_handle, 0);
	usb_close(libusb_teensy_handle);
	libusb_teensy_handle = NULL;
}

int hard_reboot(void)
{
	usb_dev_handle *rebootor;
	int r;

	rebootor = open_usb_device(0x16C0, 0x0477);

	if (!rebootor)
		rebootor = open_usb_device(0x03eb, 0x2067);

	if (!rebootor) return 0;
	r = usb_control_msg(rebootor, 0x21, 9, 0x0200, 0, "reboot", 6, 100);
	usb_release_interface(rebootor, 0);
	usb_close(rebootor);
	if (r < 0) return 0;
	return 1;
}

int soft_reboot(void)
{
	usb_dev_handle *serial_handle = NULL;

	serial_handle = open_usb_device(0x16C0, 0x0483);

	if (!serial_handle) 
		serial_handle = open_usb_device(0x03eb, 0x2067);

	if (!serial_handle) {
		char *error = usb_strerror();
		printf("Error opening USB device: %s\n", error);
		return 0;
	}

	char reboot_command[] = {0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08};
	int response = usb_control_msg(serial_handle, 0x21, 0x20, 0, 0, reboot_command, sizeof reboot_command, 10000);

	usb_release_interface(serial_handle, 0);
	usb_close(serial_handle);

	if (response < 0) {
		char *error = usb_strerror();
		printf("Unable to soft reboot with USB error: %s\n", error);
		return 0;
	}

	return 1;
}

#endif


/****************************************************************/
/*                                                              */
/*               USB Access - Microsoft WIN32                   */
/*                                                              */
/****************************************************************/

#if defined(USE_WIN32)

// http://msdn.microsoft.com/en-us/library/ms790932.aspx
#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include <ddk/hidclass.h>

HANDLE open_usb_device(int vid, int pid)
{
	GUID guid;
	HDEVINFO info;
	DWORD index, required_size;
	SP_DEVICE_INTERFACE_DATA iface;
	SP_DEVICE_INTERFACE_DETAIL_DATA *details;
	HIDD_ATTRIBUTES attrib;
	HANDLE h;
	BOOL ret;

	HidD_GetHidGuid(&guid);
	info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (info == INVALID_HANDLE_VALUE) return NULL;
	for (index=0; 1 ;index++) {
		iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		ret = SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface);
		if (!ret) {
			SetupDiDestroyDeviceInfoList(info);
			break;
		}
		SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &required_size, NULL);
		details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(required_size);
		if (details == NULL) continue;
		memset(details, 0, required_size);
		details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		ret = SetupDiGetDeviceInterfaceDetail(info, &iface, details,
			required_size, NULL, NULL);
		if (!ret) {
			free(details);
			continue;
		}
		h = CreateFile(details->DevicePath, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED, NULL);
		free(details);
		if (h == INVALID_HANDLE_VALUE) continue;
		attrib.Size = sizeof(HIDD_ATTRIBUTES);
		ret = HidD_GetAttributes(h, &attrib);
		if (!ret) {
			CloseHandle(h);
			continue;
		}
		if (attrib.VendorID != vid || attrib.ProductID != pid) {
			CloseHandle(h);
			continue;
		}
		SetupDiDestroyDeviceInfoList(info);
		return h;
	}
	return NULL;
}

int write_usb_device(HANDLE h, void *buf, int len, int timeout)
{
	static HANDLE event = NULL;
	unsigned char tmpbuf[1089];
	OVERLAPPED ov;
	DWORD n, r;

	if (len > sizeof(tmpbuf) - 1) return 0;
	if (event == NULL) {
		event = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (!event) return 0;
	}
	ResetEvent(&event);
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = event;
	tmpbuf[0] = 0;
	memcpy(tmpbuf + 1, buf, len);
	if (!WriteFile(h, tmpbuf, len + 1, NULL, &ov)) {
		if (GetLastError() != ERROR_IO_PENDING) return 0;
		r = WaitForSingleObject(event, timeout);
		if (r == WAIT_TIMEOUT) {
			CancelIo(h);
			return 0;
		}
		if (r != WAIT_OBJECT_0) return 0;
	}
	if (!GetOverlappedResult(h, &ov, &n, FALSE)) return 0;
	if (n <= 0) return 0;
	return 1;
}

void print_win32_err(void)
{
	char buf[256];
	DWORD err;

	err = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
		0, buf, sizeof(buf), NULL);
	printf("err %ld: %s\n", err, buf);
}

static HANDLE win32_teensy_handle = NULL;

int teensy_open(void)
{
	teensy_close();
	win32_teensy_handle = open_usb_device(0x16C0, 0x0478);

	if (!win32_teensy_handle)
		win32_teensy_handle = open_usb_device(0x03eb, 0x2067);


	if (win32_teensy_handle) return 1;
	return 0;
}

int teensy_write(void *buf, int len, double timeout)
{
	int r;
	uint32_t begin, now, total;

	if (!win32_teensy_handle) return 0;
	total = (uint32_t)(timeout * 1000.0);
	begin = timeGetTime();
	now = begin;
	do {
		r = write_usb_device(win32_teensy_handle, buf, len, total - (now - begin));
		if (r > 0) return 1;
		Sleep(10);
		now = timeGetTime();
	} while (now - begin < total);
	return 0;
}

void teensy_close(void)
{
	if (!win32_teensy_handle) return;
	CloseHandle(win32_teensy_handle);
	win32_teensy_handle = NULL;
}

int hard_reboot(void)
{
	HANDLE rebootor;
	int r;

	rebootor = open_usb_device(0x16C0, 0x0477);

	if (!rebootor)
		rebootor = open_usb_device(0x03eb, 0x2067);

	if (!rebootor) return 0;
	r = write_usb_device(rebootor, "reboot", 6, 100);
	CloseHandle(rebootor);
	return r;
}

int soft_reboot(void)
{
	printf("Soft reboot is not implemented for Win32\n");
	return 0;
}

#endif



/****************************************************************/
/*                                                              */
/*             USB Access - Apple's IOKit, Mac OS-X             */
/*                                                              */
/****************************************************************/

#if defined(USE_APPLE_IOKIT)

// http://developer.apple.com/technotes/tn2007/tn2187.html
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDDevice.h>

struct usb_list_struct {
	IOHIDDeviceRef ref;
	int pid;
	int vid;
	struct usb_list_struct *next;
};

static struct usb_list_struct *usb_list=NULL;
static IOHIDManagerRef hid_manager=NULL;

void attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
	CFTypeRef type;
	struct usb_list_struct *n, *p;
	int32_t pid, vid;

	if (!dev) return;
	type = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDVendorIDKey));
	if (!type || CFGetTypeID(type) != CFNumberGetTypeID()) return;
	if (!CFNumberGetValue((CFNumberRef)type, kCFNumberSInt32Type, &vid)) return;
	type = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDProductIDKey));
	if (!type || CFGetTypeID(type) != CFNumberGetTypeID()) return;
	if (!CFNumberGetValue((CFNumberRef)type, kCFNumberSInt32Type, &pid)) return;
	n = (struct usb_list_struct *)malloc(sizeof(struct usb_list_struct));
	if (!n) return;
	//printf("attach callback: vid=%04X, pid=%04X\n", vid, pid);
	n->ref = dev;
	n->vid = vid;
	n->pid = pid;
	n->next = NULL;
	if (usb_list == NULL) {
		usb_list = n;
	} else {
		for (p = usb_list; p->next; p = p->next) ;
		p->next = n;
	}
}

void detach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
	struct usb_list_struct *p, *tmp, *prev=NULL;

	p = usb_list;
	while (p) {
		if (p->ref == dev) {
			if (prev) {
				prev->next = p->next;
			} else {
				usb_list = p->next;
			}
			tmp = p;
			p = p->next;
			free(tmp);
		} else {
			prev = p;
			p = p->next;
		}
	}
}

void init_hid_manager(void)
{
	CFMutableDictionaryRef dict;
	IOReturn ret;

	if (hid_manager) return;
	hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (hid_manager == NULL || CFGetTypeID(hid_manager) != IOHIDManagerGetTypeID()) {
		if (hid_manager) CFRelease(hid_manager);
		printf_verbose("no HID Manager - maybe this is a pre-Leopard (10.5) system?\n");
		return;
	}
	dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (!dict) return;
	IOHIDManagerSetDeviceMatching(hid_manager, dict);
	CFRelease(dict);
	IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, attach_callback, NULL);
	IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, detach_callback, NULL);
	ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
	if (ret != kIOReturnSuccess) {
		IOHIDManagerUnscheduleFromRunLoop(hid_manager,
			CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		CFRelease(hid_manager);
		printf_verbose("Error opening HID Manager\n");
	}
}

static void do_run_loop(void)
{
	while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) ;
}

IOHIDDeviceRef open_usb_device(int vid, int pid)
{
	struct usb_list_struct *p;
	IOReturn ret;

	init_hid_manager();
	do_run_loop();
	for (p = usb_list; p; p = p->next) {
		if (p->vid == vid && p->pid == pid) {
			ret = IOHIDDeviceOpen(p->ref, kIOHIDOptionsTypeNone);
			if (ret == kIOReturnSuccess) return p->ref;
		}
	}
	return NULL;
}

void close_usb_device(IOHIDDeviceRef dev)
{
	struct usb_list_struct *p;

	do_run_loop();
	for (p = usb_list; p; p = p->next) {
		if (p->ref == dev) {
			IOHIDDeviceClose(dev, kIOHIDOptionsTypeNone);
			return;
		}
	}
}

static IOHIDDeviceRef iokit_teensy_reference = NULL;

int teensy_open(void)
{
	teensy_close();
	iokit_teensy_reference = open_usb_device(0x16C0, 0x0478);

	if (!iokit_teensy_reference)
		iokit_teensy_reference = open_usb_device(0x03eb, 0x2067);

	if (iokit_teensy_reference) return 1;
	return 0;
}

int teensy_write(void *buf, int len, double timeout)
{
	IOReturn ret;

	// timeouts do not work on OS-X
	// IOHIDDeviceSetReportWithCallback is not implemented
	// even though Apple documents it with a code example!
	// submitted to Apple on 22-sep-2009, problem ID 7245050
	if (!iokit_teensy_reference) return 0;

	double start = CFAbsoluteTimeGetCurrent();
	while (CFAbsoluteTimeGetCurrent() - timeout < start) {
		ret = IOHIDDeviceSetReport(iokit_teensy_reference,
			kIOHIDReportTypeOutput, 0, buf, len);
		if (ret == kIOReturnSuccess) return 1;
		usleep(10000);
	}

	return 0;
}

void teensy_close(void)
{
	if (!iokit_teensy_reference) return;
	close_usb_device(iokit_teensy_reference);
	iokit_teensy_reference = NULL;
}

int hard_reboot(void)
{
	IOHIDDeviceRef rebootor;
	IOReturn ret;

	rebootor = open_usb_device(0x16C0, 0x0477);

	if (!rebootor)
		rebootor = open_usb_device(0x03eb, 0x2067);

	if (!rebootor) return 0;
	ret = IOHIDDeviceSetReport(rebootor,
		kIOHIDReportTypeOutput, 0, (uint8_t *)("reboot"), 6);
	close_usb_device(rebootor);
	if (ret == kIOReturnSuccess) return 1;
	return 0;
}

int soft_reboot(void)
{
	printf("Soft reboot is not implemented for OSX\n");
	return 0;
}

#endif



/****************************************************************/
/*                                                              */
/*              USB Access - BSD's UHID driver                  */
/*                                                              */
/****************************************************************/

#if defined(USE_UHID)

// Thanks to Todd T Fries for help getting this working on OpenBSD
// and to Chris Kuethe for the initial patch to use UHID.

#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <dev/usb/usb.h>
#ifndef USB_GET_DEVICEINFO
#include <dev/usb/usb_ioctl.h>
#endif

int open_usb_device(int vid, int pid)
{
	int r, fd;
	DIR *dir;
	struct dirent *d;
	struct usb_device_info info;
	char buf[256];

	dir = opendir("/dev");
	if (!dir) return -1;
	while ((d = readdir(dir)) != NULL) {
		if (strncmp(d->d_name, "uhid", 4) != 0) continue;
		snprintf(buf, sizeof(buf), "/dev/%s", d->d_name);
		fd = open(buf, O_RDWR);
		if (fd < 0) continue;
		r = ioctl(fd, USB_GET_DEVICEINFO, &info);
		if (r < 0) {
			// NetBSD: added in 2004
			// OpenBSD: added November 23, 2009
			// FreeBSD: missing (FreeBSD 8.0) - USE_LIBUSB works!
			die("Error: your uhid driver does not support"
			  " USB_GET_DEVICEINFO, please upgrade!\n");
			close(fd);
			closedir(dir);
			exit(1);
		}
		//printf("%s: v=%d, p=%d\n", buf, info.udi_vendorNo, info.udi_productNo);
		if (info.udi_vendorNo == vid && info.udi_productNo == pid) {
			closedir(dir);
			return fd;
		}
		close(fd);
	}
	closedir(dir);
	return -1;
}

static int uhid_teensy_fd = -1;

int teensy_open(void)
{
	teensy_close();
	uhid_teensy_fd = open_usb_device(0x16C0, 0x0478);

	if (uhid_teensy_fd < 0)
		uhid_teensy_fd = open_usb_device(0x03eb, 0x2067);

	if (uhid_teensy_fd < 0) return 0;
	return 1;
}

int teensy_write(void *buf, int len, double timeout)
{
	int r;

	// TODO: imeplement timeout... how??
	r = write(uhid_teensy_fd, buf, len);
	if (r == len) return 1;
	return 0;
}

void teensy_close(void)
{
	if (uhid_teensy_fd >= 0) {
		close(uhid_teensy_fd);
		uhid_teensy_fd = -1;
	}
}

int hard_reboot(void)
{
	int r, rebootor_fd;

	rebootor_fd = open_usb_device(0x16C0, 0x0477);

	if (rebootor_fd < 0)
		rebootor_fd = open_usb_device(0x03eb, 0x2067);

	if (rebootor_fd < 0) return 0;
	r = write(rebootor_fd, "reboot", 6);
	delay(0.1);
	close(rebootor_fd);
	if (r == 6) return 1;
	return 0;
}

int soft_reboot(void)
{
	printf("Soft reboot is not implemented for UHID\n");
	return 0;
}

#endif



/****************************************************************/
/*                                                              */
/*                     Read Intel Hex File                      */
/*                                                              */
/****************************************************************/

// the maximum flash image size we can support
// chips with larger memory may be used, but only this
// much intel-hex data can be loaded into memory!
#define MAX_MEMORY_SIZE 0x100000

static unsigned char firmware_image[MAX_MEMORY_SIZE];
static unsigned char firmware_mask[MAX_MEMORY_SIZE];
static int end_record_seen=0;
static int byte_count;
static unsigned int extended_addr = 0;
static int parse_hex_line(char *line);

int read_intel_hex(const char *filename)
{
	FILE *fp;
	int i, lineno=0;
	char buf[1024];

	byte_count = 0;
	end_record_seen = 0;
	for (i=0; i<MAX_MEMORY_SIZE; i++) {
		firmware_image[i] = 0xFF;
		firmware_mask[i] = 0;
	}
	extended_addr = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		//printf("Unable to read file %s\n", filename);
		return -1;
	}
	while (!feof(fp)) {
		*buf = '\0';
		if (!fgets(buf, sizeof(buf), fp)) break;
		lineno++;
		if (*buf) {
			if (parse_hex_line(buf) == 0) {
				printf("Warning, HEX parse error line %d\n", lineno);
				return -2;
			}
		}
		if (end_record_seen) break;
		if (feof(stdin)) break;
	}
	fclose(fp);
	return byte_count;
}


/* from ihex.c, at http://www.pjrc.com/tech/8051/pm2_docs/intel-hex.html */

/* parses a line of intel hex code, stores the data in bytes[] */
/* and the beginning address in addr, and returns a 1 if the */
/* line was valid, or a 0 if an error occured.  The variable */
/* num gets the number of bytes that were stored into bytes[] */


int
parse_hex_line(char *line)
{
	int addr, code, num;
	int sum, len, cksum, i;
	char *ptr;

	num = 0;
	if (line[0] != ':') return 0;
	if (strlen(line) < 11) return 0;
	ptr = line+1;
	if (!sscanf(ptr, "%02x", &len)) return 0;
	ptr += 2;
	if ((int)strlen(line) < (11 + (len * 2)) ) return 0;
	if (!sscanf(ptr, "%04x", &addr)) return 0;
	ptr += 4;
	  /* printf("Line: length=%d Addr=%d\n", len, addr); */
	if (!sscanf(ptr, "%02x", &code)) return 0;
	if (addr + extended_addr + len >= MAX_MEMORY_SIZE) return 0;
	ptr += 2;
	sum = (len & 255) + ((addr >> 8) & 255) + (addr & 255) + (code & 255);
	if (code != 0) {
		if (code == 1) {
			end_record_seen = 1;
			return 1;
		}
		if (code == 2 && len == 2) {
			if (!sscanf(ptr, "%04x", &i)) return 1;
			ptr += 4;
			sum += ((i >> 8) & 255) + (i & 255);
			if (!sscanf(ptr, "%02x", &cksum)) return 1;
			if (((sum & 255) + (cksum & 255)) & 255) return 1;
			extended_addr = i << 4;
			//printf("ext addr = %05X\n", extended_addr);
		}
		if (code == 4 && len == 2) {
			if (!sscanf(ptr, "%04x", &i)) return 1;
			ptr += 4;
			sum += ((i >> 8) & 255) + (i & 255);
			if (!sscanf(ptr, "%02x", &cksum)) return 1;
			if (((sum & 255) + (cksum & 255)) & 255) return 1;
			extended_addr = i << 16;
			if (code_size > 1048576 && block_size >= 1024 &&
			   extended_addr >= 0x60000000 && extended_addr < 0x60000000 + code_size) {
				// Teensy 4.0 HEX files have 0x60000000 FlexSPI offset
				extended_addr -= 0x60000000;
			}
			//printf("ext addr = %08X\n", extended_addr);
		}
		return 1;	// non-data line
	}
	byte_count += len;
	while (num != len) {
		if (sscanf(ptr, "%02x", &i) != 1) return 0;
		i &= 255;
		firmware_image[addr + extended_addr + num] = i;
		firmware_mask[addr + extended_addr + num] = 1;
		ptr += 2;
		sum += i;
		(num)++;
		if (num >= 256) return 0;
	}
	if (!sscanf(ptr, "%02x", &cksum)) return 0;
	if (((sum & 255) + (cksum & 255)) & 255) return 0; /* checksum error */
	return 1;
}

int ihex_bytes_within_range(int begin, int end)
{
	int i;

	if (begin < 0 || begin >= MAX_MEMORY_SIZE ||
	   end < 0 || end >= MAX_MEMORY_SIZE) {
		return 0;
	}
	for (i=begin; i<=end; i++) {
		if (firmware_mask[i]) return 1;
	}
	return 0;
}

void ihex_get_data(int addr, int len, unsigned char *bytes)
{
	int i;

	if (addr < 0 || len < 0 || addr + len >= MAX_MEMORY_SIZE) {
		for (i=0; i<len; i++) {
			bytes[i] = 255;
		}
		return;
	}
	for (i=0; i<len; i++) {
		if (firmware_mask[addr]) {
			bytes[i] = firmware_image[addr];
		} else {
			bytes[i] = 255;
		}
		addr++;
	}
}

int memory_is_blank(int addr, int block_size)
{
	if (addr < 0 || addr > MAX_MEMORY_SIZE) return 1;

	while (block_size && addr < MAX_MEMORY_SIZE) {
		if (firmware_mask[addr] && firmware_image[addr] != 255) return 0;
		addr++;
		block_size--;
	}
	return 1;
}




/****************************************************************/
/*                                                              */
/*                       Misc Functions                         */
/*                                                              */
/****************************************************************/

int printf_verbose(const char *format, ...)
{
	va_list ap;
	int r;

	va_start(ap, format);
	if (verbose) {
		r = vprintf(format, ap);
		fflush(stdout);
		return r;
	}
	return 0;
}

void delay(double seconds)
{
	#ifdef WIN32
	Sleep(seconds * 1000.0);
	#else
	usleep(seconds * 1000000.0);
	#endif
}

void die(const char *str, ...)
{
	va_list  ap;

	va_start(ap, str);
	vfprintf(stderr, str, ap);
	fprintf(stderr, "\n");
	exit(1);
}

#if defined(WIN32)
#define strcasecmp stricmp
#endif


static const struct {
	const char *name;
	int code_size;
	int block_size;
} MCUs[] = {
	{"at90usb162",   15872,   128},
	{"atmega32u4",   32256,   128},
	{"at90usb646",   64512,   256},
	{"at90usb1286", 130048,   256},
#if defined(USE_LIBUSB) || defined(USE_APPLE_IOKIT) || defined(USE_WIN32)
	{"mkl26z64",     63488,   512},
	{"mk20dx128",   131072,  1024},
	{"mk20dx256",   262144,  1024},
	{"mk66fx1m0",  1048576,  1024},
	{"mk64fx512",   524288,  1024},
	{"imxrt1062",  2031616,  1024},

	// Add duplicates that match friendly Teensy Names
	// Match board names in boards.txt
	{"TEENSY",      15872,   128},
	{"TEENSYPP",    64512,   256},
	{"TEENSY2",     32256,   128},
	{"TEENSY2PP",  130048,   256},
	{"TEENSYLC",    63488,   512},
	{"TEENSY30",   131072,  1024},
	{"TEENSY31",   262144,  1024},
	{"TEENSY32",   262144,  1024},
	{"TEENSY35",   524288,  1024},
	{"TEENSY36",  1048576,  1024},
	{"TEENSY40",  2031616,  1024},
	{"TEENSY41",  8126464,  1024},
#endif
	{NULL, 0, 0},
};


void list_mcus()
{
	int i;
	printf("Supported MCUs are:\n");
	for(i=0; MCUs[i].name != NULL; i++)
		printf(" - %s\n", MCUs[i].name);
	exit(1);
}


void read_mcu(char *name)
{
	int i;

	if(name == NULL) {
		fprintf(stderr, "No MCU specified.\n");
		list_mcus();
	}

	for(i=0; MCUs[i].name != NULL; i++) {
		if(strcasecmp(name, MCUs[i].name) == 0) {
			code_size  = MCUs[i].code_size;
			block_size = MCUs[i].block_size;
			return;
		}
	}

	fprintf(stderr, "Unknown MCU type \"%s\"\n", name);
	list_mcus();
}


void parse_flag(char *arg)
{
	int i;
	for(i=1; arg[i]; i++) {
		switch(arg[i]) {
			case 'w': wait_for_device_to_appear = 1; break;
			case 'r': hard_reboot_device = 1; break;
			case 's': soft_reboot_device = 1; break;
			case 'n': reboot_after_programming = 0; break;
			case 'v': verbose = 1; break;
			case 'b': boot_only = 1; break;
			default:
				fprintf(stderr, "Unknown flag '%c'\n\n", arg[i]);
				usage(NULL);
		}
	}
}


void parse_options(int argc, char **argv)
{
	int i;
	char *arg;

	for (i=1; i<argc; i++) {
		arg = argv[i];

		//backward compatibility with previous versions.
		if(strncmp(arg, "-mmcu=", 6) == 0) {
			read_mcu(strchr(arg, '=') + 1);
		}

		else if(arg[0] == '-') {
			if(arg[1] == '-') {
				char *name = &arg[2];
				char *val  = strchr(name, '=');
				if(val == NULL) {
					//value must be the next string.
					val = argv[++i];
				}
				else {
					//we found an =, so split the string at it.
					*val = '\0';
					 val = &val[1];
				}

				if(strcasecmp(name, "help") == 0) usage(NULL);
				else if(strcasecmp(name, "mcu") == 0) read_mcu(val);
				else if(strcasecmp(name, "list-mcus") == 0) list_mcus();
				else {
					fprintf(stderr, "Unknown option \"%s\"\n\n", arg);
					usage(NULL);
				}
			}
			else parse_flag(arg);
		}
		else filename = arg;
	}
}

void boot(unsigned char *buf, int write_size)
{
	printf_verbose("Booting\n");
	memset(buf, 0, write_size);
	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	teensy_write(buf, write_size, 0.5);
}
