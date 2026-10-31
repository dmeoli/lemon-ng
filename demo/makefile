##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of the demos of LEMON                                           #
#                                                                            #
#   The compiler and the standard are chosen on the command line, e.g.,      #
#   make CC=clang++ STD=c++20.                                               #
#                                                                            #
##############################################################################

# the programs
PROGS = arg_parser_demo graph_to_eps_demo lgf_demo

# basic directory
DIR = .

# common flags
STD = c++11
COMMON_SW = -std=$(STD) -Wall -Wextra $(FLAGS)

# debug switches
SW_DEBUG = -g -DLEMON_ENABLE_DEBUG $(COMMON_SW)
# production switches
SW_RELEASE = -O2 -DNDEBUG $(COMMON_SW)

# compiler
CC = g++

# default target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

default: release

# debug target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

debug: SW = $(SW_DEBUG)
debug: $(PROGS)

# release target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

release: SW = $(SW_RELEASE)
release: $(PROGS)

# clean target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -f $(DIR)/*.o $(DIR)/*~ $(PROGS)

# define & include the necessary modules- - - - - - - - - - - - - - - - - - -

# LEMON
LEMONSDR = ..
include $(LEMONSDR)/makefile-c

# main module (linking phase) - - - - - - - - - - - - - - - - - - - - - - - -

$(PROGS): %: $(DIR)/%.o $(LEMONOBJ)
	$(CC) -o $@ $< $(LEMONOBJ) $(LEMONLIB) $(SW)

# dependencies: every .o from its .cc + every recursively included .h- - - -

$(DIR)/%.o: $(DIR)/%.cc $(LEMONH)
	$(CC) -c $< -o $@ $(LEMONINC) $(SW)

# phony targets - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

.PHONY: default debug release clean

############################ End of makefile #################################
