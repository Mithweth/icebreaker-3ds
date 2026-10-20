#---------------------------------------------------------------------------------
.SUFFIXES:
.DEFAULT_GOAL	:= all
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error Please set DEVKITARM in your environment)
endif

TOPDIR ?= $(CURDIR)
DEBUG ?= 0
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# Project configuration
#---------------------------------------------------------------------------------
BUILD				:= build
SOURCES				:= src
INCLUDES			:= include src

TARGET				:= icebreaker-3ds
APP_TITLE			:= Icebreaker
APP_DESCRIPTION		:= Icebreaker 3DS port
APP_AUTHOR			:= Jean-Baptiste Langlois
APP_PRODUCT_CODE	:= CTR-H-ICEB
APP_UNIQUE_ID		:= 0xFAA72
APP_VERSION			?= 1.0.0

ASSETS				:= assets
GRAPHICS			:= $(ASSETS)/bitmaps
SOUNDS				:= $(ASSETS)/sounds
THEMES				:= $(ASSETS)/themes
GRIDS				:= $(ASSETS)/grids
LANGUAGES			:= $(ASSETS)/lang
ROMFS				:= romfs
CIA					:= $(ASSETS)/cia

ICON				:= $(CIA)/icon.png
CIA_RSF				:= $(CIA)/app.rsf
CIA_BANNER			:= $(CIA)/banner.bnr
CIA_BANNER_PNG		:= $(CIA)/banner.png
CIA_BANNER_WAV		:= $(CIA)/banner.wav

VERSION_PARTS		:= $(subst ., ,$(APP_VERSION))
APP_VERSION_MAJOR	:= $(word 1,$(VERSION_PARTS))
APP_VERSION_MINOR	:= $(word 2,$(VERSION_PARTS))
APP_VERSION_MICRO	:= $(word 3,$(VERSION_PARTS))

# One spritesheet, loaded at runtime with C2D_SpriteSheetLoad("romfs:/gfx.t3x").
GFX_T3S				:= $(ASSETS)/bitmaps/gfx.t3s
GFX_T3X				:= $(ROMFS)/bitmaps/gfx.t3x
GFX_H				:= $(BUILD)/gfx.h
GFX_ROMFS_H			:= $(ROMFS)/bitmaps/gfx.h
GFX_IMAGES			:= $(shell find $(GRAPHICS) -type f -name '*.png' 2>/dev/null)

# Copy sounds and theme descriptions to RomFS without converting them.
RAW_ASSET_DIRS		:= $(SOUNDS) $(THEMES) $(LANGUAGES) $(GRIDS)
RAW_ASSETS			:= $(foreach dir,$(RAW_ASSET_DIRS),$(shell find $(dir) -type f 2>/dev/null))
ROMFS_ASSETS		:= $(patsubst $(ASSETS)/%,$(ROMFS)/%,$(RAW_ASSETS))

#---------------------------------------------------------------------------------
# Compiler / linker
#---------------------------------------------------------------------------------
ARCH				:= -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
LIBDIRS				:= $(CTRULIB) $(DEVKITPRO)/portlibs/3ds

INCLUDE				:=	$(foreach dir,$(INCLUDES),-I$(TOPDIR)/$(dir)) \
						$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
						-I$(TOPDIR)/$(BUILD)

CFLAGS				:= -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH) \
						-Wextra $(INCLUDE) -D__3DS__ -DVERSION=\"$(APP_VERSION)\"
ifeq ($(DEBUG),1)
CFLAGS				+= -DDEBUG
endif

CXXFLAGS			:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS				:= -g $(ARCH)
LDFLAGS				= -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS				:= -lcitro2d -lcitro3d -lctru -lm -lvorbisidec -logg

#---------------------------------------------------------------------------------
# Outer make: prepare RomFS, then compile from build/
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export TOPDIR			:= $(CURDIR)
export OUTPUT			:= $(TOPDIR)/$(TARGET)
export DEPSDIR			:= $(TOPDIR)/$(BUILD)

CFILES					:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES				:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES					:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export VPATH			:= $(foreach dir,$(SOURCES),$(TOPDIR)/$(dir))
export OFILES			:= $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES_SOURCES	:= $(OFILES)
export HFILES			:= $(TOPDIR)/$(GFX_H)
export LIBPATHS			:= $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export LD				:= $(CC)

# The 3DSX must be repackaged when RomFS contents change, even if C code doesn't.
export ROMFS_DEPS		:= $(addprefix $(TOPDIR)/,$(GFX_T3X) $(GFX_ROMFS_H) $(ROMFS_ASSETS))
export _3DSXDEPS		:= $(if $(NO_SMDH),,$(OUTPUT).smdh)
export _3DSXFLAGS		+= --romfs=$(TOPDIR)/$(ROMFS)
export APP_ICON			:= $(TOPDIR)/$(ICON)
ifeq ($(strip $(NO_SMDH)),)
export _3DSXFLAGS		+= --smdh=$(OUTPUT).smdh
endif

.PHONY: all build-project banner cia clean lint run

all: build-project

build-project: $(GFX_T3X) $(GFX_H) $(GFX_ROMFS_H) $(ROMFS_ASSETS)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(TOPDIR)/Makefile

$(BUILD) $(ROMFS):
	@mkdir -p $@

# tex3ds generates both the texture and its index header in one invocation.
$(GFX_T3X) $(GFX_H) &: $(GFX_T3S) $(GFX_IMAGES) | $(BUILD) $(ROMFS)
	@echo tex3ds ... $(GFX_T3S)
	@mkdir -p $(dir $(GFX_T3X))
	@tex3ds -i $(GFX_T3S) -H $(GFX_H) -o $(GFX_T3X)

# assets/sounds/foo.wav -> romfs/sounds/foo.wav (same for themes).
$(ROMFS)/%: $(ASSETS)/%
	@mkdir -p $(dir $@)
	@cp $< $@

$(GFX_ROMFS_H): $(GFX_H)
	@mkdir -p $(dir $@)
	@cp $< $@

banner:
	@bannertool makebanner -i $(CIA_BANNER_PNG) -a $(CIA_BANNER_WAV) -o $(CIA_BANNER)
	@echo built ... $(CIA_BANNER)

cia: $(TARGET).elf $(TARGET).smdh
	@test -f "$(CIA_BANNER)" || { \
		echo "Missing $(CIA_BANNER). Run 'make banner' first."; exit 1; \
	}
	@command -v makerom >/dev/null 2>&1 || { echo "makerom not found in PATH"; exit 1; }
	@makerom -f cia \
		-o $(TARGET).cia \
		-target t \
		-exefslogo \
		-elf $(TARGET).elf \
		-icon $(TARGET).smdh \
		-banner $(CIA_BANNER) \
		-rsf $(CIA_RSF) \
		-major $(APP_VERSION_MAJOR) \
		-minor $(APP_VERSION_MINOR) \
		-micro $(APP_VERSION_MICRO) \
		-DAPP_TITLE="$(APP_TITLE)" \
		-DAPP_PRODUCT_CODE="$(APP_PRODUCT_CODE)" \
		-DAPP_UNIQUE_ID="$(APP_UNIQUE_ID)" \
		-DAPP_ROMFS="$(TOPDIR)/$(ROMFS)" \
		-DAPP_CATEGORY="Application" \
		-DAPP_USE_ON_SD="true" \
		-DAPP_ENCRYPTED="false" \
		-DAPP_MEMORY_TYPE="Application" \
		-DAPP_SYSTEM_MODE="64MB" \
		-DAPP_SYSTEM_MODE_EXT="Legacy" \
		-DAPP_CPU_SPEED="268MHz" \
		-DAPP_ENABLE_L2_CACHE="false" \
		-DAPP_VERSION_MAJOR="$(APP_VERSION_MAJOR)"
	@echo built ... $(TARGET).cia

clean:
	@echo clean ...
	@rm -rf $(BUILD) $(ROMFS)
	@rm -f $(TARGET).3dsx $(TARGET).smdh $(TARGET).elf $(TARGET).cia PRN

lint:
	@$(MAKE) --no-print-directory CFLAGS="$(CFLAGS) -fanalyzer" build-project

run: $(TARGET).3dsx
	@flatpak info org.azahar_emu.Azahar &>/dev/null || { \
		echo "Azahar flatpak install is missing"; exit 1; \
	}
	@flatpak run --filesystem="$(TOPDIR):ro" org.azahar_emu.Azahar "$(TARGET).3dsx"

#---------------------------------------------------------------------------------
# Inner make: compile and link from build/
#---------------------------------------------------------------------------------
else

.PHONY: all
all: $(OUTPUT).3dsx

# These files already exist: the outer make generated/copied them first.
$(OFILES_SOURCES): $(HFILES)
$(OUTPUT).elf: $(OFILES)
$(OUTPUT).3dsx: $(OUTPUT).elf $(_3DSXDEPS) $(ROMFS_DEPS)

-include $(DEPSDIR)/*.d

endif
#---------------------------------------------------------------------------------
