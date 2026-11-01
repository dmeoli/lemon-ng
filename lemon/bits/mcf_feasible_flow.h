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
 * express or implied, and with no claim as to its suitability for any
 * purpose.
 *
 */

#ifndef LEMON_BITS_MCF_FEASIBLE_FLOW_H
 #define LEMON_BITS_MCF_FEASIBLE_FLOW_H

#include <cmath>
#include <limits>
#include <vector>

//\ingroup min_cost_flow_algs
//\file
//\brief The feasible flow that CostScaling and CycleCanceling start from,
//and the bound that replaces an infinite one in them and in CapacityScaling.

namespace lemon
{

namespace _mcf_bits
{

// Finds a flow within the bounds that meets the supplies on the
// residual network that CostScaling and CycleCanceling work on, which
// they used to find with Circulation on the digraph and its maps: with
// the same push-relabel algorithm (Goldberg and Tarjan, 1988), i.e., the
// active node of highest level first, every node at level zero at the
// start, a push towards any lower level and the first empty level as the
// proof that no such flow exists (Cherkassky and Goldberg, 1997), from
// either the flow that the arcs into the nodes with a deficit take
// greedily, as Circulation does, or, with warm, the flow of the last run
// put within the new bounds, so that after a change of the capacities or
// of the supplies only what the change has displaced is sent anew.
//
// It sets the residual capacities of the arcs of the digraph and of
// those of the root, the ones out of the root having root_cap if the
// supplies sum to zero, gives in sup_num the number of the nodes whose
// supply net of the lower bounds is positive, and returns false if
// there is no such flow. The last arc out of every node is the one to
// the root, which is not an arc of the digraph.
template< typename MCF >
bool feasibleFlow( MCF & mcf , bool warm , typename MCF::Value root_cap ,
		   int & sup_num )
{
 typedef typename MCF::Value Value;
 const int n = mcf._root;
 std::vector< Value > ex( mcf._supply.begin() , mcf._supply.begin() + n );
 if( mcf._has_lower ) {
  for( int u = 0 ; u != n ; ++u ) {
   const int last_out = mcf._first_out[ u + 1 ] - 1;
   for( int j = mcf._first_out[ u ] ; j != last_out ; ++j ) {
    if( ! mcf._forward[ j ] )
     continue;
    ex[ u ] -= mcf._lower[ j ];
    ex[ mcf._target[ j ] ] += mcf._lower[ j ];
    }
   }
  }
 sup_num = 0;
 for( int u = 0 ; u != n ; ++u ) {
  if( ex[ u ] > 0 )
   ++sup_num;
  }

 // The flow to start from
 for( int u = 0 ; u != n ; ++u ) {
  const int last_out = mcf._first_out[ u + 1 ] - 1;
  for( int j = mcf._first_out[ u ] ; j != last_out ; ++j ) {
   if( ! mcf._forward[ j ] )
    continue;
   const int v = mcf._target[ j ] , rj = mcf._reverse[ j ];
   const Value cap =
     mcf._has_lower ? mcf._upper[ j ] - mcf._lower[ j ] : mcf._upper[ j ];
   Value f = warm ? mcf._res_cap[ rj ] : -ex[ v ];
   if( f > cap )
    f = cap;
   if( f < 0 )
    f = 0;
   mcf._res_cap[ j ] = cap - f;
   mcf._res_cap[ rj ] = f;
   ex[ u ] -= f;
   ex[ v ] += f;
   }
  }

 // The active nodes, i.e., those with a positive excess, in a list for
 // each level, and how many nodes each level has
 std::vector< int > level( n , 0 ) , count( n + 1 , 0 ) , next( n , -1 ) ,
   head( n + 1 , -1 );
 count[ 0 ] = n;
 for( int u = 0 ; u != n ; ++u ) {
  if( ex[ u ] > 0 ) {
   next[ u ] = head[ 0 ];
   head[ 0 ] = u;
   }
  }
 for( int top = 0 ; ; ) {
  while( top >= 0 && head[ top ] < 0 )
   --top;
  if( top < 0 )
   break;
  const int u = head[ top ];
  Value e = ex[ u ];
  int low = n;
  const int last_out = mcf._first_out[ u + 1 ] - 1;
  for( int j = mcf._first_out[ u ] ; j != last_out ; ++j ) {
   const Value r = mcf._res_cap[ j ];
   const int v = mcf._target[ j ];
   if( r <= 0 || v == u )
    continue;
   const int lv = level[ v ];
   if( lv >= top ) {
    if( lv < low )
     low = lv;
    continue;
    }
   const Value d = r < e ? r : e;
   mcf._res_cap[ j ] = r - d;
   mcf._res_cap[ mcf._reverse[ j ] ] += d;
   if( ex[ v ] <= 0 && ex[ v ] + d > 0 ) {
    next[ v ] = head[ lv ];
    head[ lv ] = v;
    }
   ex[ v ] += d;
   if( ( e -= d ) == 0 )
    break;
   }
  ex[ u ] = e;
  head[ top ] = next[ u ];
  if( e == 0 )
   continue;
  // u is lifted above the lowest level it has a residual arc to, and
  // no such arc, or no node left on its level, proves that no flow
  // exists
  if( low == n || --count[ top ] == 0 )
   return( false );
  level[ u ] = ++low;
  ++count[ low ];
  next[ u ] = head[ low ];
  head[ low ] = u;
  top = low;
  }

 // The arcs of the root: with supplies that sum to less than zero, the
 // ones out of it carry what is left of the deficits
 for( int a = mcf._first_out[ n ] ; a != mcf._res_arc_num ; ++a ) {
  const int ra = mcf._reverse[ a ];
  if( mcf._sum_supply < 0 ) {
   mcf._res_cap[ a ] = -mcf._sum_supply + 1;
   mcf._res_cap[ ra ] = -ex[ mcf._target[ a ] ];
   }
  else {
   mcf._res_cap[ a ] = root_cap;
   mcf._res_cap[ ra ] = 0;
   }
  mcf._cost[ a ] = 0;
  mcf._cost[ ra ] = 0;
  }
 return( true );
 }

// Whether a cycle of negative cost is made of arcs of infinite upper bound
// (those of the digraph, not those of the root): if the problem is
// feasible, it is then unbounded. Bellman-Ford from a virtual node joined
// to all the others, on those arcs only, which still improves a distance
// after as many rounds as there are nodes if and only if there is one.
template< typename IV , typename BV , typename VV , typename CV >
bool negativeInfiniteCycle( int root , const IV & first_out ,
			    const IV & target ,
			    const BV & forward , const VV & upper ,
			    const CV & cost )
{
 typedef typename VV::value_type Value;
 typedef typename CV::value_type Cost;
 const Value MAX = ( std::numeric_limits< Value >::max )();
 bool negative = false;
 for( int u = 0 ; u != root && ! negative ; ++u )
  for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j )
   if( forward[ j ] && target[ j ] != root && upper[ j ] >= MAX &&
       cost[ j ] < 0 ) {
    negative = true;
    break;
    }
 if( ! negative )
  return( false );
 std::vector< Cost > dist( root , Cost( 0 ) );
 for( int round = 0 ; round != root ; ++round ) {
  bool changed = false;
  for( int u = 0 ; u != root ; ++u )
   for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j ) {
    const int v = target[ j ];
    if( forward[ j ] && v != root && upper[ j ] >= MAX &&
	dist[ u ] + cost[ j ] < dist[ v ] ) {
     dist[ v ] = dist[ u ] + cost[ j ];
     changed = true;
     }
    }
  if( ! changed )
   return( false );
  }
 return( true );
 }

// The bound, net of the lower bound, that replaces an infinite upper
// bound. When no cycle of negative cost is made of arcs of infinite upper
// bound, some optimal flow sends on every arc at most B beyond its lower
// bound, B being the sum of the absolute values of the supplies and of
// the finite bounds: a flow is made of paths, which carry at most the
// supplies, and of cycles, each of which has an arc of finite bound, the
// other ones having a nonnegative cost and then being of no use. One more
// than B makes an arc that reaches it keep a nonnegative reduced cost in
// an optimal solution, so that the potentials are optimal for the
// infinite bounds too. The bound is cut where the sums of the flows over
// the arcs that it replaces would no longer fit in Value.
template< typename IV , typename BV , typename VV >
typename VV::value_type infiniteBound( int root , const IV & first_out ,
				       const IV & target , const BV & forward ,
				       const VV & lower , const VV & upper ,
				       const VV & supply , bool has_lower )
{
 typedef typename VV::value_type Value;
 const Value MAX = ( std::numeric_limits< Value >::max )();
 long double b = 1;
 int inf = 0;
 for( int u = 0 ; u != root ; ++u ) {
  b += std::abs( static_cast< long double >( supply[ u ] ) );
  for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j ) {
   if( ! forward[ j ] || target[ j ] == root )
    continue;
   if( upper[ j ] < MAX )
    b += std::abs( static_cast< long double >( upper[ j ] ) );
   else
    ++inf;
   if( has_lower )
    b += std::abs( static_cast< long double >( lower[ j ] ) );
   }
  }
 const long double most = static_cast< long double >( MAX ) / ( inf + 2 );
 return( b < most ? static_cast< Value >( b )
		   : static_cast< Value >( most ) );
 }

}

}

#endif
