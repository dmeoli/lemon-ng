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
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>

#include <lemon/concept_check.h>
#include <lemon/concepts/heap.h>

#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/dijkstra.h>
#include <lemon/maps.h>

#include <lemon/bin_heap.h>
#include <lemon/quad_heap.h>
#include <lemon/dheap.h>
#include <lemon/fib_heap.h>
#include <lemon/pairing_heap.h>
#include <lemon/radix_heap.h>
#include <lemon/binomial_heap.h>
#include <lemon/bucket_heap.h>

#include "test_tools.h"

using namespace lemon;
using namespace lemon::concepts;

typedef ListDigraph Digraph;
DIGRAPH_TYPEDEFS(Digraph);

char test_lgf[] =
  "@nodes\n"
  "label\n"
  "0\n"
  "1\n"
  "2\n"
  "3\n"
  "4\n"
  "5\n"
  "6\n"
  "7\n"
  "8\n"
  "9\n"
  "@arcs\n"
  "                label   capacity\n"
  "0       5       0       94\n"
  "3       9       1       11\n"
  "8       7       2       83\n"
  "1       2       3       94\n"
  "5       7       4       35\n"
  "7       4       5       84\n"
  "9       5       6       38\n"
  "0       4       7       96\n"
  "6       7       8       6\n"
  "3       1       9       27\n"
  "5       2       10      77\n"
  "5       6       11      69\n"
  "6       5       12      41\n"
  "4       6       13      70\n"
  "3       2       14      45\n"
  "7       9       15      93\n"
  "5       9       16      50\n"
  "9       0       17      94\n"
  "9       6       18      67\n"
  "0       9       19      86\n"
  "@attributes\n"
  "source 3\n";

int test_seq[] = { 2, 28, 19, 27, 33, 25, 13, 41, 10, 26,  1,  9,  4, 34};
int test_inc[] = {20, 28, 34, 16,  0, 46, 44,  0, 42, 32, 14,  8,  6, 37};

int test_len = sizeof(test_seq) / sizeof(test_seq[0]);

template <typename Heap>
void heapSortTest() {
  RangeMap<int> map(test_len, -1);
  Heap heap(map);

  std::vector<int> v(test_len);
  for (int i = 0; i < test_len; ++i) {
    v[i] = test_seq[i];
    heap.push(i, v[i]);
  }
  std::sort(v.begin(), v.end());
  for (int i = 0; i < test_len; ++i) {
    check(v[i] == heap.prio(), "Wrong order in heap sort.");
    heap.pop();
  }
}

// Priorities of a floating-point type, many of them equal and many below
// one: the heap has to give them back in order, ties and fractions included
template <typename Heap>
void heapTiesTest() {
  const int n = 200;
  RangeMap<int> map(n, -1);
  Heap heap(map);
  std::vector<double> v(n);
  for (int i = 0; i < n; ++i) {
    v[i] = ((i * 37) % 11) / 4.0;
    heap.push(i, v[i]);
  }
  std::sort(v.begin(), v.end());
  for (int i = 0; i < n; ++i) {
    check(!heap.empty(), "Heap emptied too early.");
    check(v[i] == heap.prio(), "Wrong order with ties and fractions.");
    heap.pop();
  }
  check(heap.empty(), "Heap not emptied.");
}

// Two items of equal priority, popped one after the other: a heap that
// keeps a minimum which is no longer a root loops for ever on the second
template <typename Heap>
void heapEqualPairTest() {
  RangeMap<int> map(2, -1);
  Heap heap(map);
  heap.push(0, 0);
  heap.push(1, 0);
  heap.pop();
  heap.pop();
  check(heap.empty(), "Heap not emptied after two equal priorities.");
}

// RadixHeap after the increase of trac #681, and against a sorted set on a
// pseudo-random monotone sequence of push, pop, increase, decrease, set and
// erase: the boxes that pop() rearranges must not overlap the next ones
void radixHeapTest() {
  {
    RangeMap<int> map(3, -1);
    RadixHeap<RangeMap<int> > heap(map);
    heap.push(0, 2);
    heap.push(1, 3);
    heap.push(2, 4);
    heap.pop();
    heap.increase(1, 5);
    check(heap.top() == 2 && heap.prio() == 4, "Wrong minimum after increase.");
  }
  const int n = 40;
  RangeMap<int> map(n, -1);
  RadixHeap<RangeMap<int> > heap(map);
  std::set<std::pair<int, int> > ref;
  std::vector<int> pr(n, -1);
  int low = 0;
  unsigned int r = 12345;
  for (int step = 0; step != 4000; ++step) {
    r = r * 1103515245 + 12345;
    int i = (r >> 8) % n, op = (r >> 20) % 6, d = (r >> 4) % 100;
    if (pr[i] < 0) {
      if (op < 3) {
        pr[i] = low + d;
        heap.push(i, pr[i]);
        ref.insert(std::make_pair(pr[i], i));
      }
      continue;
    }
    ref.erase(std::make_pair(pr[i], i));
    if (op == 0) {
      pr[i] += d / 2;
      heap.increase(i, pr[i]);
    } else if (op == 1) {
      pr[i] = low + d % (pr[i] - low + 1);
      heap.decrease(i, pr[i]);
    } else if (op == 2) {
      pr[i] = low + d;
      heap.set(i, pr[i]);
    } else if (op == 3) {
      heap.erase(i);
      pr[i] = -1;
      continue;
    } else {
      ref.insert(std::make_pair(pr[i], i));
      check(heap.prio() == ref.begin()->first, "Wrong minimum in RadixHeap.");
      int t = heap.top();
      check(pr[t] == ref.begin()->first, "Wrong top in RadixHeap.");
      low = pr[t];
      ref.erase(std::make_pair(pr[t], t));
      heap.pop();
      pr[t] = -1;
      continue;
    }
    ref.insert(std::make_pair(pr[i], i));
  }
}

template <typename Heap>
void heapIncreaseTest() {
  RangeMap<int> map(test_len, -1);

  Heap heap(map);

  std::vector<int> v(test_len);
  for (int i = 0; i < test_len; ++i) {
    v[i] = test_seq[i];
    heap.push(i, v[i]);
  }
  for (int i = 0; i < test_len; ++i) {
    v[i] += test_inc[i];
    heap.increase(i, v[i]);
  }
  std::sort(v.begin(), v.end());
  for (int i = 0; i < test_len; ++i) {
    check(v[i] == heap.prio(), "Wrong order in heap increase test.");
    heap.pop();
  }
}

template <typename Heap>
void dijkstraHeapTest(const Digraph& digraph, const IntArcMap& length,
                      Node source) {

  typename Dijkstra<Digraph, IntArcMap>::template SetStandardHeap<Heap>::
    Create dijkstra(digraph, length);

  dijkstra.run(source);

  for(ArcIt a(digraph); a != INVALID; ++a) {
    Node s = digraph.source(a);
    Node t = digraph.target(a);
    if (dijkstra.reached(s)) {
      check( dijkstra.dist(t) - dijkstra.dist(s) <= length[a],
             "Error in shortest path tree.");
    }
  }

  for(NodeIt n(digraph); n != INVALID; ++n) {
    if ( dijkstra.reached(n) && dijkstra.predArc(n) != INVALID ) {
      Arc a = dijkstra.predArc(n);
      Node s = digraph.source(a);
      check( dijkstra.dist(n) - dijkstra.dist(s) == length[a],
             "Error in shortest path tree.");
    }
  }

}

int main() {

  typedef int Item;
  typedef int Prio;
  typedef RangeMap<int> ItemIntMap;

  Digraph digraph;
  IntArcMap length(digraph);
  Node source;

  std::istringstream input(test_lgf);
  digraphReader(digraph, input).
    arcMap("capacity", length).
    node("source", source).
    run();

  // BinHeap
  {
    heapTiesTest<BinHeap<double, RangeMap<int> > >();
    typedef BinHeap<Prio, ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef BinHeap<Prio, IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  // QuadHeap
  {
    heapTiesTest<QuadHeap<double, RangeMap<int> > >();
    typedef QuadHeap<Prio, ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef QuadHeap<Prio, IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  // DHeap
  {
    heapTiesTest<DHeap<double, RangeMap<int> > >();
    typedef DHeap<Prio, ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef DHeap<Prio, IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  // FibHeap
  {
    heapTiesTest<FibHeap<double, RangeMap<int> > >();
    typedef FibHeap<Prio, ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef FibHeap<Prio, IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  // PairingHeap
  {
    heapTiesTest<PairingHeap<double, RangeMap<int> > >();
    typedef PairingHeap<Prio, ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef PairingHeap<Prio, IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  // RadixHeap
  {
    radixHeapTest();
    typedef RadixHeap<ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef RadixHeap<IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  // BinomialHeap
  {
    heapTiesTest<BinomialHeap<double, RangeMap<int> > >();
    heapEqualPairTest<BinomialHeap<int, RangeMap<int> > >();
    typedef BinomialHeap<Prio, ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef BinomialHeap<Prio, IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  // BucketHeap, SimpleBucketHeap
  {
    typedef BucketHeap<ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef BucketHeap<IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);

    typedef SimpleBucketHeap<ItemIntMap> SimpleIntHeap;
    heapSortTest<SimpleIntHeap>();
  }

  {
    typedef FibHeap<Prio, ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef FibHeap<Prio, IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  {
    typedef RadixHeap<ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef RadixHeap<IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }

  {
    typedef BucketHeap<ItemIntMap> IntHeap;
    checkConcept<Heap<Prio, ItemIntMap>, IntHeap>();
    heapSortTest<IntHeap>();
    heapIncreaseTest<IntHeap>();

    typedef BucketHeap<IntNodeMap > NodeHeap;
    checkConcept<Heap<Prio, IntNodeMap >, NodeHeap>();
    dijkstraHeapTest<NodeHeap>(digraph, length, source);
  }


  return 0;
}
