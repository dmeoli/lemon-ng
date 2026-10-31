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

#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <vector>

#include <lemon/list_graph.h>
#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>

#include <lemon/network_simplex.h>
#include <lemon/capacity_scaling.h>
#include <lemon/cost_scaling.h>
#include <lemon/cycle_canceling.h>

#include <lemon/concepts/digraph.h>
#include <lemon/concepts/heap.h>
#include <lemon/concept_check.h>

#include "test_tools.h"

using namespace lemon;

// Test networks
char test_lgf[] =
  "@nodes\n"
  "label  sup1 sup2 sup3 sup4 sup5 sup6\n"
  "    1    20   27    0   30   20   30\n"
  "    2    -4    0    0    0   -8   -3\n"
  "    3     0    0    0    0    0    0\n"
  "    4     0    0    0    0    0    0\n"
  "    5     9    0    0    0    6   11\n"
  "    6    -6    0    0    0   -5   -6\n"
  "    7     0    0    0    0    0    0\n"
  "    8     0    0    0    0    0    3\n"
  "    9     3    0    0    0    0    0\n"
  "   10    -2    0    0    0   -7   -2\n"
  "   11     0    0    0    0  -10    0\n"
  "   12   -20  -27    0  -30  -30  -20\n"
  "\n"
  "@arcs\n"
  "       cost  cap low1 low2 low3\n"
  " 1  2    70   11    0    8    8\n"
  " 1  3   150    3    0    1    0\n"
  " 1  4    80   15    0    2    2\n"
  " 2  8    80   12    0    0    0\n"
  " 3  5   140    5    0    3    1\n"
  " 4  6    60   10    0    1    0\n"
  " 4  7    80    2    0    0    0\n"
  " 4  8   110    3    0    0    0\n"
  " 5  7    60   14    0    0    0\n"
  " 5 11   120   12    0    0    0\n"
  " 6  3     0    3    0    0    0\n"
  " 6  9   140    4    0    0    0\n"
  " 6 10    90    8    0    0    0\n"
  " 7  1    30    5    0    0   -5\n"
  " 8 12    60   16    0    4    3\n"
  " 9 12    50    6    0    0    0\n"
  "10 12    70   13    0    5    2\n"
  "10  2   100    7    0    0    0\n"
  "10  7    60   10    0    0   -3\n"
  "11 10    20   14    0    6  -20\n"
  "12 11    30   10    0    0  -10\n"
  "\n"
  "@attributes\n"
  "source 1\n"
  "target 12\n";

char test_neg1_lgf[] =
  "@nodes\n"
  "label   sup\n"
  "    1   100\n"
  "    2     0\n"
  "    3     0\n"
  "    4  -100\n"
  "    5     0\n"
  "    6     0\n"
  "    7     0\n"
  "@arcs\n"
  "      cost   low1   low2\n"
  "1 2    100      0      0\n"
  "1 3     30      0      0\n"
  "2 4     20      0      0\n"
  "3 4     80      0      0\n"
  "3 2     50      0      0\n"
  "5 3     10      0      0\n"
  "5 6     80      0   1000\n"
  "6 7     30      0  -1000\n"
  "7 5   -120      0      0\n";

char test_neg2_lgf[] =
  "@nodes\n"
  "label   sup\n"
  "    1   100\n"
  "    2  -300\n"
  "@arcs\n"
  "      cost\n"
  "1 2     -1\n";


// Test data
typedef ListDigraph Digraph;
DIGRAPH_TYPEDEFS(ListDigraph);

Digraph gr;
Digraph::ArcMap<int> c(gr), l1(gr), l2(gr), l3(gr), u(gr);
Digraph::NodeMap<int> s1(gr), s2(gr), s3(gr), s4(gr), s5(gr), s6(gr);
ConstMap<Arc, int> cc(1), cu(std::numeric_limits<int>::max());
Node v, w;

Digraph neg1_gr;
Digraph::ArcMap<int> neg1_c(neg1_gr), neg1_l1(neg1_gr), neg1_l2(neg1_gr);
ConstMap<Arc, int> neg1_u1(std::numeric_limits<int>::max()), neg1_u2(5000);
Digraph::NodeMap<int> neg1_s(neg1_gr);

Digraph neg2_gr;
Digraph::ArcMap<int> neg2_c(neg2_gr);
ConstMap<Arc, int> neg2_l(0), neg2_u(1000);
Digraph::NodeMap<int> neg2_s(neg2_gr);


enum SupplyType {
  EQ,
  GEQ,
  LEQ
};


// Check the interface of an MCF algorithm
template <typename GR, typename Value, typename Cost>
class McfClassConcept
{
public:

  template <typename MCF>
  struct Constraints {
    void constraints() {
      checkConcept<concepts::Digraph, GR>();

      const Constraints& me = *this;

      MCF mcf(me.g);
      const MCF& const_mcf = mcf;

      b = mcf.reset().resetParams()
             .lowerMap(me.lower)
             .upperMap(me.upper)
             .costMap(me.cost)
             .supplyMap(me.sup)
             .stSupply(me.n, me.n, me.k)
             .run();

      c = const_mcf.totalCost();
      x = const_mcf.template totalCost<double>();
      v = const_mcf.flow(me.a);
      c = const_mcf.potential(me.n);
      const_mcf.flowMap(fm);
      const_mcf.potentialMap(pm);
    }

    typedef typename GR::Node Node;
    typedef typename GR::Arc Arc;
    typedef concepts::ReadMap<Node, Value> NM;
    typedef concepts::ReadMap<Arc, Value> VAM;
    typedef concepts::ReadMap<Arc, Cost> CAM;
    typedef concepts::WriteMap<Arc, Value> FlowMap;
    typedef concepts::WriteMap<Node, Cost> PotMap;

    GR g;
    VAM lower;
    VAM upper;
    CAM cost;
    NM sup;
    Node n;
    Arc a;
    Value k;

    FlowMap fm;
    PotMap pm;
    bool b;
    double x;
    typename MCF::Value v;
    typename MCF::Cost c;
  };

};


// Check the feasibility of the given flow (primal soluiton)
template < typename GR, typename LM, typename UM,
           typename SM, typename FM >
bool checkFlow( const GR& gr, const LM& lower, const UM& upper,
                const SM& supply, const FM& flow,
                SupplyType type = EQ )
{
  TEMPLATE_DIGRAPH_TYPEDEFS(GR);

  for (ArcIt e(gr); e != INVALID; ++e) {
    if (flow[e] < lower[e] || flow[e] > upper[e]) return false;
  }

  for (NodeIt n(gr); n != INVALID; ++n) {
    typename SM::Value sum = 0;
    for (OutArcIt e(gr, n); e != INVALID; ++e)
      sum += flow[e];
    for (InArcIt e(gr, n); e != INVALID; ++e)
      sum -= flow[e];
    bool b = (type ==  EQ && sum == supply[n]) ||
             (type == GEQ && sum >= supply[n]) ||
             (type == LEQ && sum <= supply[n]);
    if (!b) return false;
  }

  return true;
}

// Check the feasibility of the given potentials (dual soluiton)
// using the "Complementary Slackness" optimality condition
template < typename GR, typename LM, typename UM,
           typename CM, typename SM, typename FM, typename PM >
bool checkPotential( const GR& gr, const LM& lower, const UM& upper,
                     const CM& cost, const SM& supply, const FM& flow,
                     const PM& pi, SupplyType type )
{
  TEMPLATE_DIGRAPH_TYPEDEFS(GR);

  bool opt = true;
  for (ArcIt e(gr); opt && e != INVALID; ++e) {
    typename CM::Value red_cost =
      cost[e] + pi[gr.source(e)] - pi[gr.target(e)];
    opt = red_cost == 0 ||
          (red_cost > 0 && flow[e] == lower[e]) ||
          (red_cost < 0 && flow[e] == upper[e]);
  }

  for (NodeIt n(gr); opt && n != INVALID; ++n) {
    typename SM::Value sum = 0;
    for (OutArcIt e(gr, n); e != INVALID; ++e)
      sum += flow[e];
    for (InArcIt e(gr, n); e != INVALID; ++e)
      sum -= flow[e];
    if (type != LEQ) {
      opt = (pi[n] <= 0) && (sum == supply[n] || pi[n] == 0);
    } else {
      opt = (pi[n] >= 0) && (sum == supply[n] || pi[n] == 0);
    }
  }

  return opt;
}

// Check whether the dual cost is equal to the primal cost
template < typename GR, typename LM, typename UM,
           typename CM, typename SM, typename PM >
bool checkDualCost( const GR& gr, const LM& lower, const UM& upper,
                    const CM& cost, const SM& supply, const PM& pi,
                    typename CM::Value total )
{
  TEMPLATE_DIGRAPH_TYPEDEFS(GR);

  typename CM::Value dual_cost = 0;
  SM red_supply(gr);
  for (NodeIt n(gr); n != INVALID; ++n) {
    red_supply[n] = supply[n];
  }
  for (ArcIt a(gr); a != INVALID; ++a) {
    if (lower[a] != 0) {
      dual_cost += lower[a] * cost[a];
      red_supply[gr.source(a)] -= lower[a];
      red_supply[gr.target(a)] += lower[a];
    }
  }

  for (NodeIt n(gr); n != INVALID; ++n) {
    dual_cost -= red_supply[n] * pi[n];
  }
  for (ArcIt a(gr); a != INVALID; ++a) {
    typename CM::Value red_cost =
      cost[a] + pi[gr.source(a)] - pi[gr.target(a)];
    dual_cost -= (upper[a] - lower[a]) * std::max(-red_cost, 0);
  }

  return dual_cost == total;
}

// Run a minimum cost flow algorithm and check the results
template < typename MCF, typename GR,
           typename LM, typename UM,
           typename CM, typename SM,
           typename PT >
void checkMcf( const MCF& mcf, PT mcf_result,
               const GR& gr, const LM& lower, const UM& upper,
               const CM& cost, const SM& supply,
               PT result, bool optimal, typename CM::Value total,
               const std::string &test_id = "",
               SupplyType type = EQ )
{
  check(mcf_result == result, "Wrong result " + test_id);
  if (optimal) {
    typename GR::template ArcMap<typename SM::Value> flow(gr);
    typename GR::template NodeMap<typename CM::Value> pi(gr);
    mcf.flowMap(flow);
    mcf.potentialMap(pi);
    check(checkFlow(gr, lower, upper, supply, flow, type),
          "The flow is not feasible " + test_id);
    check(mcf.totalCost() == total, "The flow is not optimal " + test_id);
    check(checkPotential(gr, lower, upper, cost, supply, flow, pi, type),
          "Wrong potentials " + test_id);
    check(checkDualCost(gr, lower, upper, cost, supply, pi, total),
          "Wrong dual cost " + test_id);
  }
}

// A problem in a value type narrower than int, whose arithmetic promotes
// to int: the algorithm has to compile and to solve it
template <typename MCF, typename V>
void checkNarrowType() {
  SmartDigraph g;
  SmartDigraph::Node a = g.addNode(), b = g.addNode(), c = g.addNode();
  SmartDigraph::ArcMap<V> cap(g), cost(g);
  SmartDigraph::NodeMap<V> sup(g, 0);
  SmartDigraph::Arc x = g.addArc(a, b), y = g.addArc(b, c), z = g.addArc(a, c);
  cap[x] = 3; cap[y] = 3; cap[z] = 5;
  cost[x] = 1; cost[y] = 1; cost[z] = 3;
  sup[a] = 6; sup[c] = -6;
  MCF mcf(g);
  mcf.upperMap(cap).costMap(cost).supplyMap(sup);
  check(mcf.run() == MCF::OPTIMAL && mcf.totalCost() == 15,
        "Wrong result on a narrow value type");
}

// NetworkSimplex::runWarm() after every change of a pseudo-random sequence
// of changes of the costs, of the upper bounds (some of them infinite) and
// of the supplies, against a run from scratch: the same result, and on an
// unbounded problem a cycle of negative cost and infinite capacity
template <typename V>
void checkWarmStart(int seeds) {
  typedef NetworkSimplex<SmartDigraph, V, V> NS;
  const V inf = std::numeric_limits<V>::has_infinity ?
    std::numeric_limits<V>::infinity() : (std::numeric_limits<V>::max)();
  unsigned int r = 7;
  for (int seed = 0; seed != seeds; ++seed) {
    SmartDigraph g;
    r = r * 1103515245 + 12345;
    int n = 3 + (r >> 16) % 20;
    for (int i = 0; i != n; ++i) g.addNode();
    std::vector<SmartDigraph::Arc> arcs;
    for (int k = 0; k != 3 * n; ++k) {
      r = r * 1103515245 + 12345;
      int u = (r >> 8) % n, v = (r >> 20) % n;
      if (u != v) arcs.push_back(g.addArc(g.nodeFromId(u), g.nodeFromId(v)));
    }
    if (arcs.empty()) continue;
    SmartDigraph::ArcMap<V> cap(g), cost(g);
    SmartDigraph::NodeMap<V> sup(g, 0);
    for (std::size_t k = 0; k != arcs.size(); ++k) {
      r = r * 1103515245 + 12345;
      cap[arcs[k]] = (r >> 4) % 5 == 0 ? inf : V((r >> 8) % 15);
      cost[arcs[k]] = V(int((r >> 16) % 41) - 5);
    }
    NS warm(g);
    for (int step = 0; step != 20; ++step) {
      r = r * 1103515245 + 12345;
      int what = (r >> 24) % 3;
      for (int k = 0; k != 3; ++k) {
        r = r * 1103515245 + 12345;
        SmartDigraph::Arc a = arcs[(r >> 8) % arcs.size()];
        if (what == 0) cost[a] = V(int((r >> 16) % 41) - 5);
        if (what == 1) cap[a] = (r >> 4) % 4 == 0 ? inf : V((r >> 16) % 15);
      }
      if (what == 2 || step == 0) {
        V t = 0;
        for (int i = 1; i != n; ++i) {
          r = r * 1103515245 + 12345;
          V d = V((r >> 16) % 5);
          sup[g.nodeFromId(i)] = -d;
          t += d;
        }
        sup[g.nodeFromId(0)] = t;
      }
      warm.costMap(cost).upperMap(cap).supplyMap(sup);
      typename NS::ProblemType rw = step == 0 ? warm.run() : warm.runWarm();
      NS cold(g);
      cold.costMap(cost).upperMap(cap).supplyMap(sup);
      typename NS::ProblemType rc = cold.run();
      check(rw == rc, "Wrong result of runWarm()");
      if (rw == NS::OPTIMAL) {
        check(warm.totalCost() == cold.totalCost(),
              "Wrong total cost of runWarm()");
      }
      if (rw == NS::UNBOUNDED) {
        std::vector<SmartDigraph::Arc> cyc = warm.unboundedCycle();
        V c = 0;
        for (std::size_t i = 0; i != cyc.size(); ++i) {
          c += cost[cyc[i]];
          check(cap[cyc[i]] == inf, "Wrong unbounded cycle: finite arc");
          check(g.target(cyc[i]) == g.source(cyc[(i + 1) % cyc.size()]),
                "Wrong unbounded cycle: not a cycle");
        }
        check(!cyc.empty() && c < 0, "Wrong unbounded cycle: cost");
      }
    }
  }
}

// The runWarm() of CostScaling, CycleCanceling and CapacityScaling after
// every change of a pseudo-random sequence of changes of the costs, of the
// upper bounds (some of them infinite, which make it run from scratch) and
// of the supplies, against NetworkSimplex from scratch
template <typename MCF>
void checkWarmStartOf(int seeds) {
  typedef NetworkSimplex<SmartDigraph, long> NS;
  const long inf = (std::numeric_limits<long>::max)();
  unsigned int r = 11;
  for (int seed = 0; seed != seeds; ++seed) {
    SmartDigraph g;
    r = r * 1103515245 + 12345;
    int n = 3 + (r >> 16) % 20;
    for (int i = 0; i != n; ++i) g.addNode();
    std::vector<SmartDigraph::Arc> arcs;
    for (int k = 0; k != 3 * n; ++k) {
      r = r * 1103515245 + 12345;
      int u = (r >> 8) % n, v = (r >> 20) % n;
      if (u != v) arcs.push_back(g.addArc(g.nodeFromId(u), g.nodeFromId(v)));
    }
    for (int i = 0; i + 1 < n; ++i) {
      arcs.push_back(g.addArc(g.nodeFromId(i), g.nodeFromId(i + 1)));
    }
    SmartDigraph::ArcMap<long> cap(g), cost(g);
    SmartDigraph::NodeMap<long> sup(g, 0);
    for (std::size_t k = 0; k != arcs.size(); ++k) {
      r = r * 1103515245 + 12345;
      cap[arcs[k]] = (r >> 4) % 6 == 0 ? inf : long(1 + (r >> 8) % 15);
      cost[arcs[k]] = long((r >> 16) % 41);
    }
    MCF warm(g);
    for (int step = 0; step != 20; ++step) {
      r = r * 1103515245 + 12345;
      int what = (r >> 24) % 3;
      for (int k = 0; k != 3; ++k) {
        r = r * 1103515245 + 12345;
        SmartDigraph::Arc a = arcs[(r >> 8) % arcs.size()];
        if (what == 0) cost[a] = long((r >> 16) % 41);
        if (what == 1) {
          cap[a] = (r >> 4) % 6 == 0 ? inf : long((r >> 16) % 15);
        }
      }
      if (what == 2 || step == 0) {
        long t = 0;
        for (int i = 1; i != n; ++i) {
          r = r * 1103515245 + 12345;
          long d = long((r >> 16) % 5);
          sup[g.nodeFromId(i)] = -d;
          t += d;
        }
        sup[g.nodeFromId(0)] = t;
      }
      warm.costMap(cost).upperMap(cap).supplyMap(sup);
      typename MCF::ProblemType rw = step == 0 ? warm.run() : warm.runWarm();
      NS cold(g);
      cold.costMap(cost).upperMap(cap).supplyMap(sup);
      check(int(rw) == int(cold.run()), "Wrong result of runWarm()");
      if (rw == MCF::OPTIMAL) {
        check(warm.totalCost() == cold.totalCost(),
              "Wrong total cost of runWarm()");
      }
    }
  }
}

template < typename MCF, typename Param >
void runMcfGeqTests( Param param,
                     const std::string &test_str = "",
                     bool full_neg_cost_support = false )
{
  MCF mcf1(gr), mcf2(neg1_gr), mcf3(neg2_gr);

  // Basic tests
  mcf1.upperMap(u).costMap(c).supplyMap(s1);
  checkMcf(mcf1, mcf1.run(param), gr, l1, u, c, s1,
           mcf1.OPTIMAL, true,     5240, test_str + "-1");
  mcf1.stSupply(v, w, 27);
  checkMcf(mcf1, mcf1.run(param), gr, l1, u, c, s2,
           mcf1.OPTIMAL, true,     7620, test_str + "-2");
  mcf1.lowerMap(l2).supplyMap(s1);
  checkMcf(mcf1, mcf1.run(param), gr, l2, u, c, s1,
           mcf1.OPTIMAL, true,     5970, test_str + "-3");
  mcf1.stSupply(v, w, 27);
  checkMcf(mcf1, mcf1.run(param), gr, l2, u, c, s2,
           mcf1.OPTIMAL, true,     8010, test_str + "-4");
  mcf1.resetParams().supplyMap(s1);
  checkMcf(mcf1, mcf1.run(param), gr, l1, cu, cc, s1,
           mcf1.OPTIMAL, true,       74, test_str + "-5");
  mcf1.lowerMap(l2).stSupply(v, w, 27);
  checkMcf(mcf1, mcf1.run(param), gr, l2, cu, cc, s2,
           mcf1.OPTIMAL, true,       94, test_str + "-6");
  mcf1.reset();
  checkMcf(mcf1, mcf1.run(param), gr, l1, cu, cc, s3,
           mcf1.OPTIMAL, true,        0, test_str + "-7");
  mcf1.lowerMap(l2).upperMap(u);
  checkMcf(mcf1, mcf1.run(param), gr, l2, u, cc, s3,
           mcf1.INFEASIBLE, false,    0, test_str + "-8");
  mcf1.lowerMap(l3).upperMap(u).costMap(c).supplyMap(s4);
  checkMcf(mcf1, mcf1.run(param), gr, l3, u, c, s4,
           mcf1.OPTIMAL, true,     6360, test_str + "-9");

  // Tests for the GEQ form
  mcf1.resetParams().upperMap(u).costMap(c).supplyMap(s5);
  checkMcf(mcf1, mcf1.run(param), gr, l1, u, c, s5,
           mcf1.OPTIMAL, true,     3530, test_str + "-10", GEQ);
  mcf1.lowerMap(l2);
  checkMcf(mcf1, mcf1.run(param), gr, l2, u, c, s5,
           mcf1.OPTIMAL, true,     4540, test_str + "-11", GEQ);
  mcf1.supplyMap(s6);
  checkMcf(mcf1, mcf1.run(param), gr, l2, u, c, s6,
           mcf1.INFEASIBLE, false,    0, test_str + "-12", GEQ);

  // Tests with negative costs
  mcf2.lowerMap(neg1_l1).costMap(neg1_c).supplyMap(neg1_s);
  checkMcf(mcf2, mcf2.run(param), neg1_gr, neg1_l1, neg1_u1, neg1_c, neg1_s,
           mcf2.UNBOUNDED, false,     0, test_str + "-13");
  mcf2.upperMap(neg1_u2);
  checkMcf(mcf2, mcf2.run(param), neg1_gr, neg1_l1, neg1_u2, neg1_c, neg1_s,
           mcf2.OPTIMAL, true,   -40000, test_str + "-14");
  mcf2.resetParams().lowerMap(neg1_l2).costMap(neg1_c).supplyMap(neg1_s);
  checkMcf(mcf2, mcf2.run(param), neg1_gr, neg1_l2, neg1_u1, neg1_c, neg1_s,
           mcf2.UNBOUNDED, false,     0, test_str + "-15");

  mcf3.costMap(neg2_c).supplyMap(neg2_s);
  if (full_neg_cost_support) {
    checkMcf(mcf3, mcf3.run(param), neg2_gr, neg2_l, neg2_u, neg2_c, neg2_s,
             mcf3.OPTIMAL, true,   -300, test_str + "-16", GEQ);
  } else {
    checkMcf(mcf3, mcf3.run(param), neg2_gr, neg2_l, neg2_u, neg2_c, neg2_s,
             mcf3.UNBOUNDED, false,   0, test_str + "-17", GEQ);
  }
  mcf3.upperMap(neg2_u);
  checkMcf(mcf3, mcf3.run(param), neg2_gr, neg2_l, neg2_u, neg2_c, neg2_s,
           mcf3.OPTIMAL, true,     -300, test_str + "-18", GEQ);

  // Tests for empty graph, whose empty flow is optimal
  Digraph gr0;
  MCF mcf0(gr0);
  check(mcf0.run(param) == MCF::OPTIMAL, "Wrong result for empty graph");
  check(mcf0.totalCost() == 0, "Wrong total cost");

  // Tests for a graph of one node, feasible only with a zero supply
  Digraph gr_one;
  Node n_one = gr_one.addNode();
  Digraph::NodeMap<int> sup_one(gr_one, 0);
  MCF mcf_one(gr_one);
  mcf_one.supplyMap(sup_one);
  check(mcf_one.run(param) == MCF::OPTIMAL, "Wrong result for one node");
  sup_one[n_one] = 5;
  mcf_one.supplyMap(sup_one);
  check(mcf_one.run(param) == MCF::INFEASIBLE,
        "Wrong result for one node with a supply");
}

template < typename MCF, typename Param >
void runMcfLeqTests( Param param,
                     const std::string &test_str = "" )
{
  // Tests for the LEQ form
  MCF mcf1(gr);
  mcf1.supplyType(mcf1.LEQ);
  mcf1.upperMap(u).costMap(c).supplyMap(s6);
  checkMcf(mcf1, mcf1.run(param), gr, l1, u, c, s6,
           mcf1.OPTIMAL, true,   5080, test_str + "-19", LEQ);
  mcf1.lowerMap(l2);
  checkMcf(mcf1, mcf1.run(param), gr, l2, u, c, s6,
           mcf1.OPTIMAL, true,   5930, test_str + "-20", LEQ);
  mcf1.supplyMap(s5);
  checkMcf(mcf1, mcf1.run(param), gr, l2, u, c, s5,
           mcf1.INFEASIBLE, false,  0, test_str + "-21", LEQ);
}


int main()
{
  // Read the test networks
  std::istringstream input(test_lgf);
  DigraphReader<Digraph>(gr, input)
    .arcMap("cost", c)
    .arcMap("cap", u)
    .arcMap("low1", l1)
    .arcMap("low2", l2)
    .arcMap("low3", l3)
    .nodeMap("sup1", s1)
    .nodeMap("sup2", s2)
    .nodeMap("sup3", s3)
    .nodeMap("sup4", s4)
    .nodeMap("sup5", s5)
    .nodeMap("sup6", s6)
    .node("source", v)
    .node("target", w)
    .run();

  std::istringstream neg_inp1(test_neg1_lgf);
  DigraphReader<Digraph>(neg1_gr, neg_inp1)
    .arcMap("cost", neg1_c)
    .arcMap("low1", neg1_l1)
    .arcMap("low2", neg1_l2)
    .nodeMap("sup", neg1_s)
    .run();

  std::istringstream neg_inp2(test_neg2_lgf);
  DigraphReader<Digraph>(neg2_gr, neg_inp2)
    .arcMap("cost", neg2_c)
    .nodeMap("sup", neg2_s)
    .run();

  // Check the interface of NetworkSimplex
  {
    typedef concepts::Digraph GR;
    checkConcept< McfClassConcept<GR, int, int>,
                  NetworkSimplex<GR> >();
    checkConcept< McfClassConcept<GR, double, double>,
                  NetworkSimplex<GR, double> >();
    checkConcept< McfClassConcept<GR, int, double>,
                  NetworkSimplex<GR, int, double> >();
  }

  // Check the interface of CapacityScaling
  {
    typedef concepts::Digraph GR;
    checkConcept< McfClassConcept<GR, int, int>,
                  CapacityScaling<GR> >();
    checkConcept< McfClassConcept<GR, double, double>,
                  CapacityScaling<GR, double> >();
    checkConcept< McfClassConcept<GR, int, double>,
                  CapacityScaling<GR, int, double> >();
    typedef CapacityScaling<GR>::
      SetHeap<concepts::Heap<int, RangeMap<int> > >::Create CAS;
    checkConcept< McfClassConcept<GR, int, int>, CAS >();
  }

  // Check the interface of CostScaling
  {
    typedef concepts::Digraph GR;
    checkConcept< McfClassConcept<GR, int, int>,
                  CostScaling<GR> >();
    checkConcept< McfClassConcept<GR, double, double>,
                  CostScaling<GR, double> >();
    checkConcept< McfClassConcept<GR, int, double>,
                  CostScaling<GR, int, double> >();
    typedef CostScaling<GR>::
      SetLargeCost<double>::Create COS;
    checkConcept< McfClassConcept<GR, int, int>, COS >();
  }

  // Check the interface of CycleCanceling
  {
    typedef concepts::Digraph GR;
    checkConcept< McfClassConcept<GR, int, int>,
                  CycleCanceling<GR> >();
    checkConcept< McfClassConcept<GR, double, double>,
                  CycleCanceling<GR, double> >();
    checkConcept< McfClassConcept<GR, int, double>,
                  CycleCanceling<GR, int, double> >();
  }

  // Test NetworkSimplex
  {
    typedef NetworkSimplex<Digraph> MCF;
    runMcfGeqTests<MCF>(MCF::FIRST_ELIGIBLE, "NS-FE", true);
    runMcfLeqTests<MCF>(MCF::FIRST_ELIGIBLE, "NS-FE");
    runMcfGeqTests<MCF>(MCF::BEST_ELIGIBLE,  "NS-BE", true);
    runMcfLeqTests<MCF>(MCF::BEST_ELIGIBLE,  "NS-BE");
    runMcfGeqTests<MCF>(MCF::BLOCK_SEARCH,   "NS-BS", true);
    runMcfLeqTests<MCF>(MCF::BLOCK_SEARCH,   "NS-BS");
    runMcfGeqTests<MCF>(MCF::CANDIDATE_LIST, "NS-CL", true);
    runMcfLeqTests<MCF>(MCF::CANDIDATE_LIST, "NS-CL");
    runMcfGeqTests<MCF>(MCF::ALTERING_LIST,  "NS-AL", true);
    runMcfLeqTests<MCF>(MCF::ALTERING_LIST,  "NS-AL");
  }

  // Test NetworkSimplex on floating-point supplies that sum to zero, which
  // they do in decimal and not in binary: 0.1 + 0.2 - 0.3 is 5.55e-17, and
  // the problem has to be balanced and not infeasible
  {
    SmartDigraph g;
    SmartDigraph::Node a = g.addNode(), b = g.addNode(), t = g.addNode();
    SmartDigraph::Arc at = g.addArc(a, t), bt = g.addArc(b, t);
    SmartDigraph::ArcMap<double> cost(g), cap(g);
    cost[at] = 1; cost[bt] = 2; cap[at] = cap[bt] = 1;
    SmartDigraph::NodeMap<double> sup(g);
    sup[a] = 0.1; sup[b] = 0.2; sup[t] = -0.3;
    NetworkSimplex<SmartDigraph, double> mcf(g);
    mcf.costMap(cost).upperMap(cap).supplyMap(sup);
    check(mcf.run() == mcf.OPTIMAL, "Wrong result: balanced supplies");
    check(std::abs(mcf.totalCost() - 0.5) < 1e-12,
          "Wrong total cost: balanced supplies");
  }

  // Test NetworkSimplex on floating-point costs that are mostly negative:
  // the cost of the artificial arcs has to be larger than the absolute
  // value of the cost of any path, while taken on the positive costs alone
  // it made this problem, whose optimum is -623, infeasible
  {
    const int src[] = {2, 7, 0, 1, 4, 2, 4, 3, 4, 4, 5, 4, 4};
    const int trg[] = {5, 4, 2, 8, 1, 4, 7, 0, 0, 6, 8, 1, 3};
    const double cst[] = {-1, -19, -21, -48, -37, -35, -40, -7, -2, -19, -7,
                          0, -44};
    const double cp[] = {-1, 5, -1, 2, 2, -1, 2, 1, 1, 5, 3, 2, 5};
    SmartDigraph g;
    std::vector<SmartDigraph::Node> n(9);
    for (int i = 0; i != 9; ++i) n[i] = g.addNode();
    SmartDigraph::ArcMap<double> cost(g), cap(g);
    for (int i = 0; i != 13; ++i) {
      SmartDigraph::Arc a = g.addArc(n[src[i]], n[trg[i]]);
      cost[a] = cst[i];
      cap[a] = cp[i] < 0 ? std::numeric_limits<double>::infinity() : cp[i];
    }
    SmartDigraph::NodeMap<double> sup(g, 0);
    sup[n[0]] = 4; sup[n[8]] = -4;
    NetworkSimplex<SmartDigraph, double> mcf(g);
    mcf.costMap(cost).upperMap(cap).supplyMap(sup);
    check(mcf.run() == mcf.OPTIMAL, "Wrong result: negative costs");
    check(mcf.totalCost() == -623, "Wrong total cost: negative costs");
  }

  // Test CostScaling and CycleCanceling run again after the supplies have
  // grown, on arcs with no upper bound: the bound that a run puts in place
  // of the infinite one, the sum of the supplies, must not stay for the
  // next run, which would find the larger supplies infeasible
  {
    SmartDigraph g;
    SmartDigraph::Node n[3];
    for (int i = 0; i != 3; ++i) n[i] = g.addNode();
    SmartDigraph::ArcMap<int> cost(g);
    for (int i = 0; i != 2; ++i) cost[g.addArc(n[i], n[i + 1])] = 1;
    SmartDigraph::NodeMap<int> sup(g, 0);
    sup[n[0]] = 4; sup[n[2]] = -4;
    CostScaling<SmartDigraph> cs(g);
    CycleCanceling<SmartDigraph> cc(g);
    cs.costMap(cost).supplyMap(sup);
    cc.costMap(cost).supplyMap(sup);
    check(cs.run() == cs.OPTIMAL && cs.totalCost() == 8,
          "Wrong result: CostScaling, no upper bounds");
    check(cc.run() == cc.OPTIMAL && cc.totalCost() == 8,
          "Wrong result: CycleCanceling, no upper bounds");
    sup[n[0]] = 40; sup[n[2]] = -40;
    cs.supplyMap(sup);
    cc.supplyMap(sup);
    check(cs.run() == cs.OPTIMAL && cs.totalCost() == 80,
          "Wrong result: CostScaling, no upper bounds, supplies grown");
    check(cc.run() == cc.OPTIMAL && cc.totalCost() == 80,
          "Wrong result: CycleCanceling, no upper bounds, supplies grown");
  }

  // Test the four algorithms on value types narrower than int
  {
    typedef SmartDigraph G;
    checkNarrowType<NetworkSimplex<G, short>, short>();
    checkNarrowType<CostScaling<G, short>, short>();
    checkNarrowType<CycleCanceling<G, short>, short>();
    checkNarrowType<CapacityScaling<G, short>, short>();
    checkNarrowType<CapacityScaling<G, signed char>, signed char>();
  }

  // Test CostScaling on a chain with costs of both signs, along which the
  // ranks of the price refinement sum up past the number of its buckets:
  // the chain has one feasible flow, of cost -99
  {
    const int sup[] = {7, -2, -3, 2, 1, 1, -1, 1, 2, 0, -7, 6, 2, -9};
    const int cap[] = {14, 10, 4, 8, 10, 12, 10, 12, 16, 16, 2, 14, 18};
    const int cst[] = {-1, -3, 0, 0, -3, -2, -3, -1, 0, 1, -3, -1, -3};
    SmartDigraph g;
    std::vector<SmartDigraph::Node> v(14);
    for (int i = 0; i != 14; ++i) v[i] = g.addNode();
    std::vector<SmartDigraph::Arc> a(13);
    for (int i = 0; i != 13; ++i) a[i] = g.addArc(v[i], v[i + 1]);
    SmartDigraph::ArcMap<int> c(g), u(g);
    SmartDigraph::NodeMap<int> s(g);
    for (int i = 0; i != 13; ++i) {
      c[a[i]] = cst[i];
      u[a[i]] = cap[i];
    }
    for (int i = 0; i != 14; ++i) s[v[i]] = sup[i];
    typedef CostScaling<SmartDigraph> CS;
    const CS::Method method[] = {CS::PUSH, CS::AUGMENT, CS::PARTIAL_AUGMENT};
    for (int m = 0; m != 3; ++m) {
      CS cs(g);
      cs.costMap(c).upperMap(u).supplyMap(s);
      check(cs.run(method[m]) == CS::OPTIMAL && cs.totalCost() == -99,
            "Wrong result of CostScaling on a chain of signed costs");
    }
  }

  // Test the warm start of NetworkSimplex
  checkWarmStart<long>(200);
  checkWarmStart<double>(200);
  checkWarmStartOf<CostScaling<SmartDigraph, long> >(100);
  checkWarmStartOf<CycleCanceling<SmartDigraph, long> >(100);
  checkWarmStartOf<CapacityScaling<SmartDigraph, long> >(100);

  // Test CapacityScaling
  {
    typedef CapacityScaling<Digraph> MCF;
    runMcfGeqTests<MCF>(0, "SSP");
    runMcfGeqTests<MCF>(2, "CAS");
  }

  // Test CostScaling
  {
    typedef CostScaling<Digraph> MCF;
    runMcfGeqTests<MCF>(MCF::PUSH, "COS-PR");
    runMcfGeqTests<MCF>(MCF::AUGMENT, "COS-AR");
    runMcfGeqTests<MCF>(MCF::PARTIAL_AUGMENT, "COS-PAR");
  }

  // Test CycleCanceling
  {
    typedef CycleCanceling<Digraph> MCF;
    runMcfGeqTests<MCF>(MCF::SIMPLE_CYCLE_CANCELING, "SCC");
    runMcfGeqTests<MCF>(MCF::MINIMUM_MEAN_CYCLE_CANCELING, "MMCC");
    runMcfGeqTests<MCF>(MCF::CANCEL_AND_TIGHTEN, "CAT");
  }

  return 0;
}
