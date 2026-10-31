##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of LEMON                                                        #
#                                                                            #
#   The makefile defines the objects of the library of LEMON, the -I         #
#   directives to compile what includes its headers and the headers          #
#   themselves; lemon/config.h, which CMake writes from lemon/config.h.in,   #
#   is written here in $(LEMONSDR)/obj/lemon. The libraries that LEMON       #
#   needs (the threads of the system) are added by makefile-c, which is      #
#   the makefile to include from a project that uses LEMON.                  #
#                                                                            #
#   Input:  $(CC)          = compiler command                                #
#           $(SW)          = compiler options                                #
#           $(LEMONSDR)    = the directory where the source is               #
#           $(LEMON_VERSION) = the version written in lemon/config.h (by     #
#                            default the tag of git, or its commit)          #
#                                                                            #
#   Output: $(LEMONOBJ)    = the final object(s)                             #
#           $(LEMONH)      = the .h files to include                         #
#           $(LEMONINC)    = the -I$( source directory )                     #
#                                                                            #
##############################################################################

# macros to be exported - - - - - - - - - - - - - - - - - - - - - - - - - - -

LEMONOBJ = $(LEMONSDR)/obj/arg_parser.o $(LEMONSDR)/obj/base.o \
	$(LEMONSDR)/obj/color.o $(LEMONSDR)/obj/lp_base.o \
	$(LEMONSDR)/obj/lp_skeleton.o $(LEMONSDR)/obj/random.o \
	$(LEMONSDR)/obj/bits/windows.o

LEMONINC = -I$(LEMONSDR) -I$(LEMONSDR)/obj

LEMONH = $(wildcard $(LEMONSDR)/lemon/*.h $(LEMONSDR)/lemon/bits/*.h \
	$(LEMONSDR)/lemon/concepts/*.h) $(LEMONSDR)/obj/lemon/config.h

# internal macros - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

LEMON_VERSION ?= $(shell git -C $(LEMONSDR) describe --tags --always \
	2> /dev/null || echo unknown)

# the threads of the system: Win32 on Windows, POSIX elsewhere
ifeq ($(OS),Windows_NT)
LEMON_SYS_SED = -e 's/^\#cmakedefine LEMON_WIN32 1/\#define LEMON_WIN32 1/' \
	-e 's/^\#cmakedefine \(LEMON_USE_WIN32_THREADS\) 1/\#define \1 1/'
else
LEMON_SYS_SED = -e 's/^\#cmakedefine \(LEMON_USE_PTHREAD\) 1/\#define \1 1/'
endif

# clean - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -rf $(LEMONSDR)/obj $(LEMONSDR)/*~

# lemon/config.h, as CMake writes it for GCC and Clang with no LP or MIP
# solver: the version, long long, the threads, the pragma that keeps quiet
# the unused local types, every other #cmakedefine left undefined - - - - - -

$(LEMONSDR)/obj/lemon/config.h: $(LEMONSDR)/lemon/config.h.in
	mkdir -p $(dir $@)
	sed -e 's/@PROJECT_VERSION@/$(LEMON_VERSION)/' \
	  -e 's/^#cmakedefine \(LEMON_HAVE_LONG_LONG\) 1/#define \1 1/' \
	  -e 's/^#cmakedefine \(LEMON_NO_UNUSED_[A-Z_]*\) 1/#define \1 1/' \
	  $(LEMON_SYS_SED) \
	  -e 's/^#cmakedefine \([A-Za-z0-9_]*\).*/\/* #undef \1 *\//' $< > $@

# dependencies: every .o from its .cc + every recursively included .h - - - -

$(LEMONSDR)/obj/%.o: $(LEMONSDR)/lemon/%.cc $(LEMONH)
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(LEMONINC) $(SW)

########################## End of makefile ###################################
