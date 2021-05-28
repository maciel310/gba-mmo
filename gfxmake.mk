#
# Making a gfx library from grit output
#
# For use on data-arrays only. NO CODE!!!
#

# ---------------------------------------------------------------------
# SETUP
# ---------------------------------------------------------------------

export PATH :=  $(DEVKITARM)/bin:$(PATH)

.SUFFIXES:

include $(DEVKITARM)/base_rules


# ---------------------------------------------------------------------
# (1) PROJECT DETAILS
# ---------------------------------------------------------------------

# GFXTITLE  : Graphics library name
# BUILD	 : Directory for build process temporaries. Should NOT be empty!
# GFXDIRS   : List of graphics directories
# GFXEXTS   : Graphics extensions.
# General note: use . for the current dir, don't leave them empty.

BUILD	   := build
GFXDIRS	 := gfx/map gfx/sprites gfx/menus
GFXLIB	  ?= libgfx.a
GFXHDR	  ?= all_gfx.h

GFXEXTS	 := png bmp

# --- Exceptions ---
# Add files/file-variables for special rules here. Put the rules 
# At the bottom of the makefile. Be careful with directories, as
# we'll be in $(BUILD) when converting.
# GFXSPECIALS   : removed from GFXFILES
# OSPECIALS	 : added to OFILES

export SPRITES	?= $(notdir $(wildcard gfx/sprites/*.png))

# Key exception variables
export GFXSPECIALS  := $(SPRITES)
OSPECIALS		   := sprites.o


# ---------------------------------------------------------------------
# BUILD FLAGS
# ---------------------------------------------------------------------

# Since there's no code to compile, we won't need optimizations, 
# architectures etc.

CFLAGS	  := 
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS	 := 


# ---------------------------------------------------------------------
# (2) BUILD PROCEDURE
# ---------------------------------------------------------------------

ifneq ($(BUILD),$(notdir $(CURDIR)))

# still in main directory.

export TARGET   :=  $(CURDIR)/$(BUILD)/$(GFXLIB)

export VPATH	:=  $(foreach dir, $(GFXDIRS), $(CURDIR)/$(dir))
export DEPSDIR  :=  $(CURDIR)/$(BUILD)

GFXFILES	:= $(filter-out $(GFXSPECIALS),		 \
	$(foreach dir, $(GFXDIRS),					  \
		$(foreach ext, $(GFXEXTS),				  \
			$(notdir $(wildcard $(dir)/*.$(ext)))   \
	)))

export OFILES   := $(addsuffix .o, $(basename $(GFXFILES))) $(OSPECIALS)


# --- More targets ----------------------------------------------------

.PHONY: $(BUILD) clean

# --- Create BUILD if necessary, and run this makefile from there ---

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/gfxmake.mk

all : $(BUILD)

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET) $(GFXHDR)

	
# ---------------------------------------------------------------------

else

DEPENDS :=  $(OFILES:.o=.d)

.PHONY : all 

all : $(TARGET) $(GFXHDR)

$(TARGET) : $(OFILES)
	@echo Archiving into $(notdir $@)
	-@rm -f $@
	@$(AR) rcs $@ $(OFILES)


$(GFXHDR) : $(OFILES)
	@echo "Creating master header: $@"
	@$(call master-header, $@, $(notdir $(^:.o=.h)) )


# ---------------------------------------------------------------------
# (3) BASE CONVERSION RULES
# ---------------------------------------------------------------------

# --- With separate .grit file ---

%.s %.h : %.png %.grit
	grit $< -fts

%.s %.h : %.bmp %.grit
	grit $< -fts

%.s %.h : %.pcx %.grit
	grit $< -fts

%.s %.h : %.jpg %.grit
	grit $< -fts


# --- Without .grit file ; uses dirname/dirname.grit for options ---

%.s %.h : %.png
	grit $< -fts -ff $(<D)/$(notdir $(<D)).grit

%.s %.h : %.bmp
	grit $< -fts -ff $(<D)/$(notdir $(<D)).grit

%.s %.h : %.pcx
	grit $< -fts -ff $(<D)/$(notdir $(<D)).grit

%.s %.h : %.jpg
	grit $< -fts -ff $(<D)/$(notdir $(<D)).grit


# ---------------------------------------------------------------------
# (4) SPECIAL RULES 
# ---------------------------------------------------------------------

sprites.s sprites.h : sprites.grit $(SPRITES)
	@echo $(notdir $^)
	grit $(sort $(filter %.png,$^)) -o$@ -ff $<


# ---------------------------------------------------------------------
# (5) UTILITY FUNCTIONS
# ---------------------------------------------------------------------

## Merge all headers into a single large one for easier including.
define master-header	# $1 : master path, $2 separate header paths
	echo "//\n// $(notdir $(strip $1))\n//" > $1
	echo "// One header to rule them and in the darkness bind them" >> $1
	echo "// Date: $(shell date +'%F %X' )\n" >> $1
	echo "#ifdef __cplusplus\nextern \"C\" {\n#endif" >> $1
	cat $2 >> $1
	echo "\n#ifdef __cplusplus\n};\n#endif\n" >> $1
endef

## if you just want to include the separate headers, use this instead of cat:
#   for hdr in $2 ; \
#	   do echo -e "#include \"$$hdr\"" >> $1 ; done;

# --- odds and ends ---

## Get the title-part of filename.
define title		# $1: filepath
	$(basename $(notdir $1))
endef

## Get a valid C identifier for a name.
define cident	   # $1: name
	`echo $1 | sed -e 's|^\([0-9]\)|_\1| ; s|[./\\-]|_|g'`
endef

## Create a header file for a bin2s converted binary.
define bin-header	   # $1: path, $2: identifier
	echo "extern const u32 $(strip $2)_size;" >  $1
	echo "extern const u8 $(strip $2)[];"	 >> $1 
endef

# ---------------------------------------------------------------------
# DEPENDENCIES
# ---------------------------------------------------------------------

-include $(DEPENDS)

endif
