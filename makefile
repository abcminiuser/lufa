#
#             LUFA Library
#     Copyright (C) Dean Camera, 2012.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# -----------------------------------
#  Sample LUFA Application Makefile.
# -----------------------------------

MCU    = at90usb1287
ARCH   = AVR8
BOARD  = USBKEY
F_CPU  = 8000000
F_USB  = $(F_CPU)
OPT    = s
TARGET = Target
SRC    = Target.c

LUFA_PATH = .

%: all

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa.build.in
include $(LUFA_PATH)/Build/lufa.doxygen.in
include $(LUFA_PATH)/Build/lufa.dfu.in