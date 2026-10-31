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

// Another library with a function named as the helper of LEMON, which it
// calls unqualified, as Boost does: the call on a type of LEMON must not
// find the helper of LEMON by argument-dependent lookup and be ambiguous.

#include <lemon/list_graph.h>
#include <lemon/concept_check.h>

#include "test_tools.h"

namespace other
{

int calls = 0;

template< class T >
inline void ignore_unused_variable_warning( const T & )
{
 ++calls;
 }

template< class T >
void touch( const T & t )
{
 ignore_unused_variable_warning( t );
 }

}

int main()
{
 lemon::ListDigraph g;
 lemon::ListDigraph::Node n = g.addNode();
 other::touch( n );
 check( other::calls == 1 , "The function of the other library not called" );
 ::lemon::ignore_unused_variable_warning( n , g );
 return( 0 );
 }
