#
#            DMBS Build System
#     Released into the public domain.
#
#  dean [at] fourwalledcubicle [dot] com
#        www.fourwalledcubicle.com
#

DMBS_BUILD_MODULES         += AVRDUDE
DMBS_BUILD_TARGETS         += avrdude-lfuse avrdude-hfuse avrdude-efuse avrdude-lock avrdude-fuses
DMBS_BUILD_TARGETS         += avrdude avrdude-ee avrdude-all avrdude-all-ee
DMBS_BUILD_MANDATORY_VARS  += MCU TARGET
DMBS_BUILD_OPTIONAL_VARS   += AVRDUDE_PROGRAMMER AVRDUDE_PORT AVRDUDE_FLAGS AVRDUDE_MEMORY AVRDUDE_BAUD
DMBS_BUILD_OPTIONAL_VARS   += AVRDUDE_LFUSE AVRDUDE_HFUSE AVRDUDE_EUSE AVRDUDE_LOCK
DMBS_BUILD_PROVIDED_VARS   +=
DMBS_BUILD_PROVIDED_MACROS +=

# Import the CORE module of DMBS
DMBS_MODULE_PATH := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
include $(DMBS_MODULE_PATH)/core.mk

# Default values of optionally user-supplied variables
AVRDUDE_PROGRAMMER ?= jtagicemkii
AVRDUDE_PORT       ?= usb
AVRDUDE_FLAGS      ?=
AVRDUDE_MEMORY     ?= flash
AVRDUDE_LFUSE      ?=
AVRDUDE_HFUSE      ?=
AVRDUDE_EFUSE      ?=
AVRDUDE_LOCK       ?=
AVRDUDE_BAUD       ?=

# Sanity check user supplied values
$(foreach MANDATORY_VAR, $(DMBS_BUILD_MANDATORY_VARS), $(call ERROR_IF_UNSET, $(MANDATORY_VAR)))
$(call ERROR_IF_EMPTY, MCU)
$(call ERROR_IF_EMPTY, TARGET)
$(call ERROR_IF_EMPTY, AVRDUDE_PROGRAMMER)
$(call ERROR_IF_EMPTY, AVRDUDE_PORT)

# Output Messages
MSG_AVRDUDE_CMD    := ' [AVRDUDE] :'

# Construct base avrdude command flags
BASE_AVRDUDE_FLAGS := -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
ifneq ($(AVRDUDE_BAUD),)
  BASE_AVRDUDE_FLAGS += -b $(AVRDUDE_BAUD)
endif

# Programs in the target FLASH memory using AVRDUDE
avrdude: $(TARGET).hex $(MAKEFILE_LIST)
	@echo $(MSG_AVRDUDE_CMD) Programming device \"$(MCU)\" FLASH using \"$(AVRDUDE_PROGRAMMER)\" on port \"$(AVRDUDE_PORT)\"
	avrdude $(BASE_AVRDUDE_FLAGS) -U $(AVRDUDE_MEMORY):w:$< $(AVRDUDE_FLAGS)

# Programs in the target EEPROM memory using AVRDUDE
avrdude-ee: $(TARGET).eep $(MAKEFILE_LIST)
	@echo $(MSG_AVRDUDE_CMD) Programming device \"$(MCU)\" EEPROM using \"$(AVRDUDE_PROGRAMMER)\" on port \"$(AVRDUDE_PORT)\"
	avrdude $(BASE_AVRDUDE_FLAGS) -U eeprom:w:$< $(AVRDUDE_FLAGS)

# Programs in the target fuses using AVRDUDE
avrdude-lfuse: $(MAKEFILE_LIST)
	@echo $(MSG_AVRDUDE_CMD) Programming device \"$(MCU)\" low fuse using \"$(AVRDUDE_PROGRAMMER)\" on port \"$(AVRDUDE_PORT)\"
	$(call ERROR_IF_EMPTY, AVRDUDE_LFUSE)
	avrdude $(BASE_AVRDUDE_FLAGS) -Ulfuse:w:$(AVRDUDE_LFUSE):m $(AVRDUDE_FLAGS)

avrdude-hfuse: $(MAKEFILE_LIST)
	@echo $(MSG_AVRDUDE_CMD) Programming device \"$(MCU)\" high fuse using \"$(AVRDUDE_PROGRAMMER)\" on port \"$(AVRDUDE_PORT)\"
	$(call ERROR_IF_EMPTY, AVRDUDE_HFUSE)
	avrdude $(BASE_AVRDUDE_FLAGS) -Uhfuse:w:$(AVRDUDE_HFUSE):m $(AVRDUDE_FLAGS)

avrdude-efuse: $(MAKEFILE_LIST)
	@echo $(MSG_AVRDUDE_CMD) Programming device \"$(MCU)\" extended fuse using \"$(AVRDUDE_PROGRAMMER)\" on port \"$(AVRDUDE_PORT)\"
	$(call ERROR_IF_EMPTY, AVRDUDE_EFUSE)
	avrdude $(BASE_AVRDUDE_FLAGS) -Uefuse:w:$(AVRDUDE_EFUSE):m $(AVRDUDE_FLAGS)

avrdude-lock: $(MAKEFILE_LIST)
	@echo $(MSG_AVRDUDE_CMD) Programming device \"$(MCU)\" lock bits using \"$(AVRDUDE_PROGRAMMER)\" on port \"$(AVRDUDE_PORT)\"
	$(call ERROR_IF_EMPTY, AVRDUDE_LOCK)
	avrdude $(BASE_AVRDUDE_FLAGS) -Ulock:w:$(AVRDUDE_LOCK):m $(AVRDUDE_FLAGS)

avrdude-fuses: avrdude-lfuse avrdude-hfuse avrdude-efuse avrdude-lock

avrdude-all: avrdude avrdude-fuses

avrdude-all-ee: avrdude avrdude-ee avrdude-fuses

# Phony build targets for this module
.PHONY: $(DMBS_BUILD_TARGETS)
