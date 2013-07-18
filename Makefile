#---------------------------------------------------------------------------------
# Generic makefile for Gamecube projects
#
# Tab stops set to 4
#	|	|	|	|
#	0	1	2	3
#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	redux
BUILD		:=	build
SOURCES		:=	src/fileio src src/cdaudio src/cdrom src/z80i \
			src/memory src/pd4990a src/cpu src/input src/video \
			src/mcard src/sound src/tffs

INCLUDES	:=	src/z80 src/m68000 src/cpu src/fileio src \
			src/bin src/cdaudio src/cdrom src/memory src/z80i \
			src/video src/pd4990a src/input src/mcard \
			src/sound src/tffs

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
MACHDEP	= -DGEKKO -mcpu=750 -meabi -mhard-float 
CFLAGS  = -g -O2 -Wall $(MACHDEP) $(INCLUDE) \
	  -Wno-sign-compare -Wundef -fstrict-aliasing \
	  -fomit-frame-pointer \
	  -Wdisabled-optimization -funsigned-char \
	  -Wpointer-arith -Wcast-align -Waggregate-return \
	  -Wbad-function-cast -Wshadow \
	  -Wstrict-prototypes -Wformat-security -Wwrite-strings \
	  -DUSESD

LDFLAGS	=	$(MACHDEP) -mogc -Wl,-Map,$(notdir $@).map -Wl,--cref

PREFIX	:=	powerpc-gekko-

#export PATH:=/c/devkitPPC_r11/bin:/bin

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS := 	

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with
#---------------------------------------------------------------------------------
LIBS	:=	-logc -lm -lz -logcsys -lsdcard -lbba -ltinysmb \
		-lmad -L./ -lz80 -lmc68000 -ldb

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export CC		:=	$(PREFIX)gcc
export CXX		:=	$(PREFIX)g++
export AR		:=	$(PREFIX)ar
export OBJCOPY	:=	$(PREFIX)objcopy
#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:= $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir $@
	@cd src/m68000 && $(MAKE)
	@cd src/z80 && $(MAKE)
	@echo ""
	@echo "*** NeoCD Redux 0.1.52A ****************************************************"
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@echo "*************************************************************** Yay! \o/ ***"
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf
	@cd src/m68000 && $(MAKE) clean
	@cd src/z80 && $(MAKE) clean

#---------------------------------------------------------------------------------
run:
	psoload $(TARGET).dol

#---------------------------------------------------------------------------------
reload:
	psoload -r $(TARGET).dol

#---------------------------------------------------------------------------------
dist:
	@touch neocdredux-0.1.52A.tar.bz2
	@rm neocdredux-0.1.52A.tar.bz2 2>/dev/null
	@tar jcvf neocdredux-0.1.52A.tar.bz2 src/* doc/* redux.dol Makefile --exclude .svn \
	 --exclude neocd.bin

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
	@echo "Output    ... "$(notdir $@)
	@$(OBJCOPY)  -O binary $< $@

#---------------------------------------------------------------------------------
$(OUTPUT).elf: $(OFILES)
	@echo "Linking   ... "$(notdir $@)
	@$(LD)  $^ $(LDFLAGS) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# Compile Targets for C/C++
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o : %.cpp
	@echo Compiling ... $(notdir $<)
	@$(CXX) -MMD $(CFLAGS) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.c
	@echo Compiling ... $(notdir $<)
	@$(CC) -MMD $(CFLAGS) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.S
	@echo Compiling ... $(notdir $<)
	@$(CC) -MMD $(CFLAGS) -D_LANGUAGE_ASSEMBLY -c $< -o $@

#---------------------------------------------------------------------------------
%.o : %.s
	@echo Compiling ... $(notdir $<)
	@$(CC) -MMD $(CFLAGS) -D_LANGUAGE_ASSEMBLY -c $< -o $@

#---------------------------------------------------------------------------------
# canned command sequence for binary data
#---------------------------------------------------------------------------------
define bin2o
	cp $(<) $(*).tmp
	$(OBJCOPY) -I binary -O elf32-powerpc -B powerpc \
	--rename-section .data=.rodata,readonly,data,contents,alloc \
	--redefine-sym _binary_$*_tmp_start=$*\
	--redefine-sym _binary_$*_tmp_end=$*_end\
	--redefine-sym _binary_$*_tmp_size=$*_size\
	$(*).tmp $(@)
	echo "extern const u8" $(*)"[];" > $(*).h
	echo "extern const u32" $(*)_size[]";" >> $(*).h
	rm $(*).tmp
endef

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
