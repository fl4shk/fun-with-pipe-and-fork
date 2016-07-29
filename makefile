# Edit these variables if more directories are needed.  Order is C++, C, ASM, NASM.
# Separate each entry by spaces.
CXX_DIRS=$(CURDIR) 
C_DIRS=$(CXX_DIRS) 
S_DIRS=$(CXX_DIRS) 

# NASM Source Directories
# Note:  NS is a prefix for NASM in all cases
NS_DIRS=$(CXX_DIRS) 


# This is likely specific to *nix.
PROJ=$(shell basename $(CURDIR))


PREFIX=
CC=$(PREFIX)gcc
CXX=$(PREFIX)g++
AS=$(PREFIX)as
NS=nasm
LD=$(PREFIX)g++


#DEBUG=yeah do debug

#DEBUG_OPTIMIZATION_LEVEL=-O1
DEBUG_OPTIMIZATION_LEVEL=-Og
#DEBUG_OPTIMIZATION_LEVEL=-O2

REGULAR_OPTIMIZATION_LEVEL=-O2
#REGULAR_OPTIMIZATION_LEVEL=-O3


ifdef DEBUG
	DEBUG_FLAGS=-gdwarf-3 -g
	
	#BASE_FLAGS=-masm=intel -march=native -Wall $(DEBUG_OPTIMIZATION_LEVEL) -g
	#BASE_FLAGS=-masm=intel -Wall $(DEBUG_OPTIMIZATION_LEVEL) -g
	BASE_FLAGS=-Wall $(DEBUG_OPTIMIZATION_LEVEL) -g
else
	#BASE_FLAGS=-masm=intel -march=native -Wall $(REGULAR_OPTIMIZATION_LEVEL)
	#BASE_FLAGS=-masm=intel -Wall $(REGULAR_OPTIMIZATION_LEVEL)
	BASE_FLAGS=-Wall $(REGULAR_OPTIMIZATION_LEVEL)
endif


CXX_FLAGS=-std=c++14 $(BASE_FLAGS)
C_FLAGS=-std=c11 $(BASE_FLAGS)
S_FLAGS=-mnaked-reg -msyntax=intel
NS_FLAGS=-f elf64

LD_FLAGS=-lm $(DEBUG_FLAGS)



OBJDIR=objs
DEPDIR=deps
OBJDIR_TEMP=objs_temp

CXX_SOURCES=$(foreach DIR,$(CXX_DIRS),$(notdir $(wildcard $(DIR)/*.cpp)))
C_SOURCES=$(foreach DIR,$(C_DIRS),$(notdir $(wildcard $(DIR)/*.c)))
S_SOURCES=$(foreach DIR,$(S_DIRS),$(notdir $(wildcard $(DIR)/*.s)))
NS_SOURCES=$(foreach DIR,$(S_DIRS),$(notdir $(wildcard $(DIR)/*.nasm)))

export VPATH	:=	$(foreach DIR,$(CXX_DIRS),$(CURDIR)/$(DIR)) \
	$(foreach DIR,$(C_DIRS),$(CURDIR)/$(DIR)) \
	$(foreach DIR,$(S_DIRS),$(CURDIR)/$(DIR)) \
	$(foreach DIR,$(NS_DIRS),$(CURDIR)/$(DIR))


CXX_OFILES=$(patsubst %.cpp,$(OBJDIR)/%.o,$(CXX_SOURCES))
C_OFILES=$(patsubst %.c,$(OBJDIR)/%.o,$(C_SOURCES))
S_OFILES=$(patsubst %.s,$(OBJDIR)/%.o,$(S_SOURCES))
NS_OFILES=$(patsubst %.nasm,$(OBJDIR)/%.o,$(NS_SOURCES))
OFILES=$(CXX_OFILES) $(C_OFILES) $(S_OFILES) $(NS_OFILES)

CXX_PFILES=$(patsubst %.cpp,$(DEPDIR)/%.P,$(CXX_SOURCES))
C_PFILES=$(patsubst %.c,$(DEPDIR)/%.P,$(C_SOURCES))
S_PFILES=$(patsubst %.s,$(DEPDIR)/%.P,$(S_SOURCES))
NS_PFILES=$(patsubst %.nasm,$(DEPDIR)/%.P,$(NS_SOURCES))
PFILES=$(CXX_PFILES) $(C_PFILES) $(S_PFILES) $(NS_PFILES)

CXX_OFILES_TEMP=$(patsubst %.cpp,$(OBJDIR_TEMP)/%.o,$(CXX_SOURCES))
C_OFILES_TEMP=$(patsubst %.c,$(OBJDIR_TEMP)/%.o,$(C_SOURCES))
S_OFILES_TEMP=$(patsubst %.s,$(OBJDIR_TEMP)/%.o,$(S_SOURCES))
NS_OFILES_TEMP=$(patsubst %.nasm,$(OBJDIR_TEMP)/%.o,$(NS_SOURCES))
OFILES_TEMP=$(CXX_OFILES_TEMP) $(C_OFILES_TEMP) $(S_OFILES_TEMP) $(NS_OFILES_TEMP)


all : all_pre $(OFILES)
	$(LD) $(OBJDIR)/*.o -o $(PROJ) $(LD_FLAGS)

all_objs : all_pre $(OFILES)
	@#

all_pre :
	mkdir -p $(OBJDIR) $(DEPDIR)

$(CXX_OFILES) : $(OBJDIR)/%.o : %.cpp
	@#echo "Generating dependency information for "$@"...."
	@echo $@" was updated or has no object file.  (Re)Compiling...."
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@
	@cp $(OBJDIR)/$*.d $(DEPDIR)/$*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(OBJDIR)/$*.d >> $(DEPDIR)/$*.P
	@rm -f $(OBJDIR)/$*.d

$(C_OFILES) : $(OBJDIR)/%.o : %.c
	@#echo "Generating dependency information for "$@"...."
	@echo $@" was updated or has no object file.  (Re)Compiling...."
	$(CC) $(C_FLAGS) -MMD -c $< -o $@
	@cp $(OBJDIR)/$*.d $(DEPDIR)/$*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(OBJDIR)/$*.d >> $(DEPDIR)/$*.P
	@rm -f $(OBJDIR)/$*.d
	
# For NASM sources, the dependency generation is somewhat different from
# that of C/C++ sources.
$(NS_OFILES) : $(OBJDIR)/%.o : %.nasm
	@#echo "Generating dependency information for "$@"...."
	@echo $@" was updated or has no object file.  (Re)Assembling...."
	$(NS) $(NS_FLAGS) $< -o $@ -MD $(DEPDIR)/$*.P
	
-include $(PFILES)


#¯\(°_o)/¯

.PHONY : clean
clean :
	rm -rfv $(OBJDIR) $(DEPDIR) $(PROJ) tags *.taghl

.PHONY : clean_objs_with_no_source
clean_objs_with_no_source :
	@mkdir -p $(OBJDIR_TEMP)
	@#ls $(OBJDIR)
	@echo "Removing object files that don't have corresponding source files...."
	@for objfile in $(OFILES); \
	do \
		if [ -f $$objfile ]; \
		then \
			mv $$objfile $(OBJDIR_TEMP); \
		fi; \
	done;
	@#ls $(OBJDIR_TEMP)
	@rm -rf $(OBJDIR)
	@mkdir -p $(OBJDIR)
	@for objfile in $(OFILES_TEMP); \
	do \
		if [ -f $$objfile ]; \
		then \
			mv $$objfile $(OBJDIR); \
		fi; \
	done;
	@#ls $(OBJDIR)
	@rmdir $(OBJDIR_TEMP)
	
	
	
	@#rm -rfv $(OBJDIR_TEMP)



# Note that -j8 is used on my laptop because its CPU is quad core with
# hyperthreading.
.PHONY : check_build
check_build :
	make clean_objs_with_no_source && make -j8


