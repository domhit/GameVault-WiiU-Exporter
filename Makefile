.SUFFIXES:
ifeq ($(strip $(DEVKITPRO)),)
$(error "DEVKITPRO is not set")
endif
ifeq ($(strip $(DEVKITPPC)),)
$(error "DEVKITPPC is not set")
endif
TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/wut/share/wut_rules
TARGET := GameVault-WiiU-Exporter
BUILD := build
SOURCES := source
INCLUDES := include
LDFLAGS = -g $(ARCH) $(RPXSPECS) -Wl,-Map,$(notdir $*.map)
LIBS := -liosuhax -lmocha -lwut
LIBDIRS := $(WUT_ROOT) $(WUT_ROOT)/usr
ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(BUILD)/$(TARGET)
export TOPDIR := $(CURDIR)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export LD := $(CXX)
export OFILES := $(CPPFILES:.cpp=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections $(MACHDEP) $(INCLUDE) -D__WIIU__ -D__WUT__
export CXXFLAGS := $(CFLAGS) -std=gnu++20 -fno-exceptions -fno-rtti
export ASFLAGS := -g $(ARCH)
.PHONY: all clean $(BUILD)
all: $(BUILD)
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@mkdir -p $(BUILD)/release
	@cp $(BUILD)/$(TARGET).wuhb $(BUILD)/release/$(TARGET).wuhb
clean:
	@rm -rf $(BUILD)
else
DEPENDS := $(OFILES:.o=.d)
.PHONY: all
all: $(OUTPUT).wuhb
$(OUTPUT).wuhb: $(OUTPUT).rpx
$(OUTPUT).rpx: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(DEPENDS)
endif
