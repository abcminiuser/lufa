OS ?= LINUX
#OS ?= WINDOWS
#OS ?= MACOSX
#OS ?= BSD

# uncomment this to use libusb on Macintosh, instead of Apple's HID manager via IOKit
# this is technically not the "correct" way to support Macs, but it's been reported to
# work.
#USE_LIBUSB ?= YES

ifeq ($(OS), LINUX)  # also works on FreeBSD
CC ?= gcc
CFLAGS ?= -O2 -Wall
hid_bootloader_cli: hid_bootloader_cli.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DUSE_LIBUSB -o hid_bootloader_cli hid_bootloader_cli.c -lusb $(LDFLAGS)


else ifeq ($(OS), WINDOWS)
CC ?= i586-mingw32msvc-gcc
CFLAGS ?= -O2 -Wall
hid_bootloader_cli.exe: hid_bootloader_cli.c
	$(CC) $(CFLAGS) -s -DUSE_WIN32 -o hid_bootloader_cli.exe hid_bootloader_cli.c -lhid -lsetupapi -lwinmm


else ifeq ($(OS), MACOSX)
ifeq ($(USE_LIBUSB), YES)
CC ?= gcc
CFLAGS ?= -O2 -Wall
hid_bootloader_cli: hid_bootloader_cli.c
	$(CC) $(CFLAGS) -DUSE_LIBUSB -DMACOSX -o hid_bootloader_cli hid_bootloader_cli.c -lusb -I /usr/local/include -L/usr/local/lib
	 
else
CC ?= gcc
SDK ?= $(shell xcrun --show-sdk-path)
#SDK ?= /Developer/SDKs/MacOSX10.6.sdk  # the old way...
#SDK = /Developer_xcode32/SDKs/MacOSX10.5.sdk  # the very old way!
#CC = /Developer_xcode32/usr/bin/gcc-4.0
#CFLAGS = -O2 -Wall -arch i386 -arch ppc
CFLAGS ?= -O2 -Wall
hid_bootloader_cli: hid_bootloader_cli.c
ifeq ($(SDK),)
	$(error SDK was not found. To use this type of compilation please install Xcode)
endif
	$(CC) $(CFLAGS) -DUSE_APPLE_IOKIT -isysroot $(SDK) -o hid_bootloader_cli hid_bootloader_cli.c -Wl,-syslibroot,$(SDK) -framework IOKit -framework CoreFoundation

endif

else ifeq ($(OS), BSD)  # works on NetBSD and OpenBSD
CC ?= gcc
CFLAGS ?= -O2 -Wall
hid_bootloader_cli: hid_bootloader_cli.c
	$(CC) $(CFLAGS) -s -DUSE_UHID -o hid_bootloader_cli hid_bootloader_cli.c


endif


clean:
	rm -f hid_bootloader_cli hid_bootloader_cli.exe*
