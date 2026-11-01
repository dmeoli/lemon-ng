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

// A program built against an installed static LEMON that has LP and MIP
// solvers: it links their libraries only through the configuration of
// LEMON, which looks for them again on this machine, and solves with each
// of them a problem whose optimum it knows.

#include <iostream>
#include <lemon/config.h>
#include <lemon/lp_base.h>
#ifdef LEMON_HAVE_GLPK
 #include <lemon/glpk.h>
#endif
#ifdef LEMON_HAVE_CLP
 #include <lemon/clp.h>
#endif
#ifdef LEMON_HAVE_CBC
 #include <lemon/cbc.h>
#endif

// max x with 0 <= x <= 3, whose optimum is 3
template< class Lp >
bool lp_ok( const char * name )
{
 Lp lp;
 lemon::LpBase::Col x = lp.addCol();
 lp.colLowerBound( x , 0 );
 lp.colUpperBound( x , 3 );
 lp.obj( x );
 lp.max();
 lp.solve();
 std::cout << name << ": " << lp.primal() << std::endl;
 return( lp.primal() == 3 );
 }

// max x with 0 <= x <= 2.5 and x integer, whose optimum is 2
template< class Mip >
bool mip_ok( const char * name )
{
 Mip mip;
 lemon::LpBase::Col x = mip.addCol();
 mip.colType( x , Mip::INTEGER );
 mip.colLowerBound( x , 0 );
 mip.colUpperBound( x , 2.5 );
 mip.obj( x );
 mip.max();
 mip.solve();
 std::cout << name << ": " << mip.solValue() << std::endl;
 return( mip.solValue() == 2 );
 }

int main()
{
 int solvers = 0;
 bool ok = true;
#ifdef LEMON_HAVE_GLPK
 ok = lp_ok< lemon::GlpkLp >( "GlpkLp" ) && ok;
 ok = mip_ok< lemon::GlpkMip >( "GlpkMip" ) && ok;
 solvers += 2;
#endif
#ifdef LEMON_HAVE_CLP
 ok = lp_ok< lemon::ClpLp >( "ClpLp" ) && ok;
 ++solvers;
#endif
#ifdef LEMON_HAVE_CBC
 ok = mip_ok< lemon::CbcMip >( "CbcMip" ) && ok;
 ++solvers;
#endif
 if( solvers == 0 )
  std::cout << "this LEMON has no LP solver" << std::endl;
 return( ok && solvers > 0 ? 0 : 1 );
 }
