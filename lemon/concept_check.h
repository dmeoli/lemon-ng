/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2013
 * Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
 * (Egervary Research Group on Combinatorial Optimization, EGRES).
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

// The contents of this file was inspired by the concept checking
// utility of the BOOST library (http://www.boost.org).

///\file
///\brief Basic utilities for concept checking.
///

#ifndef LEMON_CONCEPT_CHECK_H
 #define LEMON_CONCEPT_CHECK_H

namespace lemon
{

namespace _concept_bits
{

// A function object rather than overloaded functions, so that the
// unqualified calls that other libraries (e.g., Boost) make to their own
// function of the same name do not find this one by argument-dependent
// lookup on the types of LEMON and become ambiguous
struct IgnoreUnusedVariableWarning
{
 template< typename... T >
 void operator()( const T &... ) const { }
 };

}

///\e
const _concept_bits::IgnoreUnusedVariableWarning
  ignore_unused_variable_warning =
    _concept_bits::IgnoreUnusedVariableWarning();

///\e
template< class Concept >
inline void function_requires()
{
#if ! defined( NDEBUG )
 void ( Concept::*x )() = &Concept::constraints;
 ::lemon::ignore_unused_variable_warning( x );
#endif
 }

///\e
template< typename Concept , typename Type >
inline void checkConcept()
{
#if ! defined( NDEBUG )
 typedef typename Concept::template Constraints< Type > ConceptCheck;
 void ( ConceptCheck::*x )() = &ConceptCheck::constraints;
 ::lemon::ignore_unused_variable_warning( x );
#endif
 }

}  // namespace lemon

#endif  // LEMON_CONCEPT_CHECK_H
