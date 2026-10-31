<p align="center">
  <img src="doc/theme/lemon-logo.png" alt="lemon-ng" width="60">
</p>

# lemon-ng

[![CI](https://github.com/dmeoli/lemon-ng/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/dmeoli/lemon-ng/actions/workflows/ci.yml)
[![Packaging](https://github.com/dmeoli/lemon-ng/actions/workflows/packaging.yml/badge.svg?branch=master)](https://github.com/dmeoli/lemon-ng/actions/workflows/packaging.yml)
[![Release](https://img.shields.io/github/v/release/dmeoli/lemon-ng)](https://github.com/dmeoli/lemon-ng/releases)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://dmeoli.github.io/lemon-ng/)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11%20to%2023-blue)](https://en.cppreference.com/w/cpp)
[![Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)](https://github.com/dmeoli/lemon-ng/actions/workflows/ci.yml)
[![Homebrew](https://img.shields.io/badge/homebrew-dmeoli%2Flemon--ng-orange)](https://github.com/dmeoli/homebrew-lemon-ng)
[![License: BSL-1.0](https://img.shields.io/badge/License-BSL_1.0-blue.svg)](https://www.boost.org/LICENSE_1_0.txt)

lemon-ng is a maintained fork of [LEMON](https://lemon.cs.elte.hu/trac/lemon),
the Library for Efficient Modeling and Optimization in Networks, i.e., a C++
template library of graphs, maps and heaps and of the algorithms of network
optimization built on them (shortest paths, maximum and minimum cost flows,
matchings, connectivity, cuts and cycles), written by the Egerváry Research
Group of the Eötvös Loránd University of Budapest. LEMON has had no release
since 1.3.1 (2014); hence, the fixes collected in the meantime by its trac and
by its forks were never distributed, and its headers no longer compile with the
compilers and the standards of today (e.g., C++20 removed the members of
`std::allocator` that `ArrayMap` and `ListPath` call). Its documentation is at
<https://dmeoli.github.io/lemon-ng/>. lemon-ng starts from the last revision of
the Mercurial repository of LEMON, whose history it keeps. Its headers are
still `lemon/` and its namespace is still `lemon`; hence, a program written for
LEMON may be compiled with lemon-ng unchanged (and, possibly, with fewer
warnings).

## Differences from LEMON 1.3.1

**Portability.** With GCC, Clang, AppleClang and MSVC, the library
compiles without warnings from C++11 to C++23. In particular, the allocators go
through `std::allocator_traits`, the iterators no longer derive from the
deprecated `std::iterator`, and the dynamic exception specifications are
`noexcept`. Also, the headers survive the macros `min` and `max` of
`<windows.h>` and `MIN` and `MAX` of `<sys/param.h>`, and each of them
is self-contained.

**Correctness.** Several defects of the algorithms and of the data
structures are fixed, most of them with a test that failed before the fix.
For instance, `RadixHeap` mixed the items of adjacent boxes (the old code
was wrong on 18892 of 20000 random sequences of operations), and
`BinomialHeap` truncated fractional priorities and could loop in `pop()`.
Furthermore, `Vf2` and `Vf2pp` indexed a map with `INVALID`, `CbcMip`
reported a proven infeasible problem as feasible, and `BellmanFord` could
delete a map of the user. Default-constructed, the items of the graphs are
now `INVALID`, and the fields that some classes copied while
still uninitialized are now initialized.

**Minimum cost flow.** `NetworkSimplex` solves problems with floating-point
costs and capacities, on which it typically pivoted for ever or reported
infeasibility, since it compared reduced costs and residual flows against an
exact zero. It also updates the potentials on the smaller side of the tree at
each pivot, and it gives the cycle that certifies an unbounded problem
(`unboundedCycle()`). `CostScaling` and `CycleCanceling` find their starting
flow on their residual network, and `CapacityScaling` uses a 4-ary heap. Each
of the four algorithms has `runWarm()`, which solves the problem again after a
change of the costs, of the upper bounds or of the supplies: it starts from the
basis, the flow or the potentials of the last run, and it does what `run()`
does when the last run did not end at an optimum. Also, they answer that a
digraph without nodes has an optimal (empty) flow.

**Building and installing.** The build with CMake (3.19 or later) honors
`CMAKE_CXX_STANDARD`, takes the version from the tag of git, and can leave
out the tests, the demos, the tools and the documentation; what it installs
(a CMake configuration with the target `LEMON::lemon`, and `lemon.pc`) may
be moved to another prefix. Makefiles build the library, the tests, the
tools and the demos with GNU make and no configuration step. Each change is
built and tested by the continuous integration on Linux, macOS and Windows,
with the compilers above, under the address and undefined behavior
sanitizers, with the makefiles and from an installation.

The fixes contributed to LEMON by Peter Madarasi (`BinomialHeap`, `Vf2`),
Jörg Winkler (self-contained headers), Bernhard X. Kausler (the iterable
maps), Martin Heistermann (`WinLock`), rturrado (the LGF readers under C++23)
and Fredrik Bräutigam (CMake) are in the history with their authors, whose log
lists all the changes.

## Building and installing

With CMake, as for LEMON (see [INSTALL](INSTALL) for the options):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
cmake --install build --prefix /usr/local
```

With GNU make, without a configuration step:

```sh
make -f makefile-lib            # lib/libemon.a
make -C test check              # the tests, built and run
```

A CMake project finds the installed library with

```cmake
find_package(LEMON CONFIG REQUIRED)
target_link_libraries(main PRIVATE LEMON::lemon)
```

while other build systems can use `pkg-config --cflags --libs lemon`. A
project built with make can instead include `makefile-c`, which defines the
objects, the `-I` directives and the headers of the library (see
[INSTALL](INSTALL)).

## Source tree

`lemon/` holds the library and `test/` the programs that check it. `tools/`
has a generator of random graphs, a converter from the DIMACS formats and a
solver of DIMACS instances, and `demo/` has a few programs to start from.
In `doc/` is the documentation, which Doxygen builds with the target `html`
of CMake, and `scripts/` has the tools of the development (among them, the
formatter of the code).

## Code style

Its code follows the `.clang-format` at the root, with what
`scripts/format.py` adds to it (e.g., `return( x );` and a space before each
comma), which clang-format has no option for. On a pull request, the
continuous integration checks the lines that a pull request changes, and

```sh
python3 scripts/format.py --since origin/master FILE...
```

puts them in the style. This needs clang-format 22 (e.g., `pip install
clang-format==22.1.5`); `scripts/ide-format` does the same on the file that
an IDE shows.

## License

lemon-ng is distributed, as LEMON, under the Boost Software License 1.0
(see [LICENSE](LICENSE)).
