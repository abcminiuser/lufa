#
#             LUFA Library
#     Copyright (C) Dean Camera, 2009.
#              
#  dean [at] fourwalledcubicle [dot] com
#      www.fourwalledcubicle.com
#

# Makefile to build all the LUFA Demos. Call with "make all" to
# rebuild all demos.

# Projects are pre-cleaned before each one is built, to ensure any
# custom LUFA library build options are reflected in the compiled
# code.

all:
	make -C Mouse clean
	make -C Mouse all

	make -C TestApp clean
	make -C TestApp all

%:
	make -C Mouse $@
	make -C TestApp $@
