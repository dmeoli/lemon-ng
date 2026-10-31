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

#include <vector>

//\ingroup min_cost_flow_algs
//\file
//\brief The feasible flow that CostScaling and CycleCanceling start from.

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

}

}

#endif
