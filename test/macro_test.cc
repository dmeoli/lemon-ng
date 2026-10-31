/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2026 the authors of lemon-ng.
 *
 * Permission to use, modify and distribute this software is granted
 * provided that this copyright notice appears in all copies. For
 * precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind,
 * express or implied, and with no claim concerning its usefulness
 * for any purpose.
 *
 */

// The headers have to compile when the function-like macros min and max
// of <windows.h> (without NOMINMAX) and MIN and MAX of <sys/param.h> on BSD
// and macOS are defined before them, after the standard headers, which on
// those systems are written to bear them.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#define min( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define max( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

#include <lemon/adaptors.h>
#include <lemon/arg_parser.h>
#include <lemon/assert.h>
#include <lemon/bellman_ford.h>
#include <lemon/bfs.h>
#include <lemon/bin_heap.h>
#include <lemon/binomial_heap.h>
#include <lemon/bucket_heap.h>
#include <lemon/capacity_scaling.h>
#include <lemon/christofides_tsp.h>
#include <lemon/circulation.h>
#include <lemon/color.h>
#include <lemon/compact_graph.h>
#include <lemon/concept_check.h>
#include <lemon/connectivity.h>
#include <lemon/core.h>
#include <lemon/cost_scaling.h>
#include <lemon/counter.h>
#include <lemon/cycle_canceling.h>
#include <lemon/dfs.h>
#include <lemon/dheap.h>
#include <lemon/dijkstra.h>
#include <lemon/dim2.h>
#include <lemon/dimacs.h>
#include <lemon/edge_set.h>
#include <lemon/edmonds_karp.h>
#include <lemon/elevator.h>
#include <lemon/error.h>
#include <lemon/euler.h>
#include <lemon/fib_heap.h>
#include <lemon/fractional_matching.h>
#include <lemon/full_graph.h>
#include <lemon/gomory_hu.h>
#include <lemon/graph_to_eps.h>
#include <lemon/greedy_tsp.h>
#include <lemon/grid_graph.h>
#include <lemon/grosso_locatelli_pullan_mc.h>
#include <lemon/hao_orlin.h>
#include <lemon/hartmann_orlin_mmc.h>
#include <lemon/howard_mmc.h>
#include <lemon/hypercube_graph.h>
#include <lemon/insertion_tsp.h>
#include <lemon/karp_mmc.h>
#include <lemon/kruskal.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/list_graph.h>
#include <lemon/lp_base.h>
#include <lemon/maps.h>
#include <lemon/matching.h>
#include <lemon/math.h>
#include <lemon/max_cardinality_search.h>
#include <lemon/min_cost_arborescence.h>
#include <lemon/nagamochi_ibaraki.h>
#include <lemon/nauty_reader.h>
#include <lemon/nearest_neighbor_tsp.h>
#include <lemon/network_simplex.h>
#include <lemon/opt2_tsp.h>
#include <lemon/pairing_heap.h>
#include <lemon/path.h>
#include <lemon/planarity.h>
#include <lemon/preflow.h>
#include <lemon/quad_heap.h>
#include <lemon/radix_heap.h>
#include <lemon/radix_sort.h>
#include <lemon/random.h>
#include <lemon/smart_graph.h>
#include <lemon/static_graph.h>
#include <lemon/suurballe.h>
#include <lemon/time_measure.h>
#include <lemon/tolerance.h>
#include <lemon/unionfind.h>
#include <lemon/vf2.h>
#include <lemon/vf2pp.h>

#include "test_tools.h"

using namespace lemon;

template< typename MCF >
void checkMcf()
{
 SmartDigraph g;
 SmartDigraph::Node a = g.addNode() , b = g.addNode();
 g.addArc( a , b );
 SmartDigraph::ArcMap< int > cost( g , 2 );
 SmartDigraph::NodeMap< int > sup( g , 0 );
 sup[ a ] = 3;
 sup[ b ] = -3;
 MCF mcf( g );
 mcf.costMap( cost ).supplyMap( sup );
 check( mcf.run() == MCF::OPTIMAL && mcf.totalCost() == 6 ,
	"Wrong result with the macros min, max, MIN and MAX defined" );
 }

int main()
{
 checkMcf< NetworkSimplex< SmartDigraph > >();
 checkMcf< CostScaling< SmartDigraph > >();
 checkMcf< CycleCanceling< SmartDigraph > >();
 checkMcf< CapacityScaling< SmartDigraph > >();
 return( 0 );
 }
