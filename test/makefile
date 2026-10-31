##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of the tests of LEMON                                           #
#                                                                            #
#   Builds the tests that CMake builds (the ones of the LP and MIP solvers   #
#   excepted), and runs them with                                            #
#                                                                            #
#       make check                                                           #
#                                                                            #
#   which stops at the first that fails. The compiler and the standard are   #
#   chosen on the command line, e.g., make CC=clang++ STD=c++20 check.       #
#                                                                            #
##############################################################################

# the tests
TESTS = adaptors_test arc_look_up_test bellman_ford_test bfs_test \
	bpgraph_test circulation_test concept_check_test \
	connectivity_test counter_test dfs_test digraph_test \
	dijkstra_test dim_test edge_set_test error_test euler_test \
	fractional_matching_test gomory_hu_test graph_copy_test \
	graph_test graph_utils_test hao_orlin_test heap_test \
	kruskal_test lgf_reader_writer_test lgf_test macro_test \
	maps_test matching_test max_cardinality_search_test \
	max_clique_test max_flow_test min_cost_arborescence_test \
	min_cost_flow_test min_mean_cycle_test nagamochi_ibaraki_test \
	path_test planarity_test radix_sort_test random_test \
	suurballe_test time_measure_test tsp_test unionfind_test \
	vf2_test

# basic directory
DIR = .

# common flags
STD = c++11
COMMON_SW = -std=$(STD) -Wall -Wextra $(FLAGS)

# debug switches
SW_DEBUG = -g -DLEMON_ENABLE_DEBUG $(COMMON_SW)
# production switches
SW_RELEASE = -O2 $(COMMON_SW)

# compiler
CC = g++

# default target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

default: release

# debug target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

debug: SW = $(SW_DEBUG)
debug: $(TESTS)

# release target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

release: SW = $(SW_RELEASE)
release: $(TESTS)

# check target: every test run, the first that fails stops it- - - - - - - -

check: release
	@for t in $(TESTS) ; do \
	  ./$$t > /dev/null || { echo "$$t FAILED" ; exit 1 ; } ; \
	  echo "$$t passed" ; \
	done

# clean target- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -f $(DIR)/*.o $(DIR)/*~ $(TESTS)

# define & include the necessary modules- - - - - - - - - - - - - - - - - - -

# LEMON
LEMONSDR = ..
include $(LEMONSDR)/makefile-c

# main module (linking phase) - - - - - - - - - - - - - - - - - - - - - - - -

$(TESTS): %: $(DIR)/%.o $(LEMONOBJ)
	$(CC) -o $@ $< $(LEMONOBJ) $(LEMONLIB) $(SW)

# dependencies: every .o from its .cc + every recursively included .h- - - -

$(DIR)/%.o: $(DIR)/%.cc $(DIR)/test_tools.h $(LEMONH)
	$(CC) -c $< -o $@ $(LEMONINC) $(SW)

# phony targets - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

.PHONY: default debug release check clean

############################ End of makefile #################################
