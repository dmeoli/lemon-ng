class LemonNg < Formula
  desc "Maintained fork of LEMON, a C++ library of graphs and network optimization"
  homepage "https://github.com/dmeoli/lemon-ng"
  url "https://github.com/dmeoli/lemon-ng/archive/refs/tags/1.4.0.tar.gz"
  sha256 "6cd337ef985bbce1d1c8515c7ddc8ce1df24ece8e51b6b817c89eb346d3cd34c"
  license "BSL-1.0"
  head "https://github.com/dmeoli/lemon-ng.git", branch: "master"

  depends_on "cmake" => :build

  def install
    args = %W[
      -DBUILD_SHARED_LIBS=ON
      -DLEMON_VERSION=#{version}
      -DLEMON_BUILD_TESTS=NO
      -DLEMON_BUILD_DEMOS=NO
      -DLEMON_BUILD_TOOLS=NO
      -DLEMON_BUILD_DOC=NO
      -DLEMON_ENABLE_GLPK=NO
      -DLEMON_ENABLE_ILOG=NO
      -DLEMON_ENABLE_COIN=NO
      -DLEMON_ENABLE_SOPLEX=NO
    ]
    system "cmake", "-S", ".", "-B", "build", *args, *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    (testpath/"test.cpp").write <<~CPP
      #include <lemon/network_simplex.h>
      #include <lemon/smart_graph.h>
      int main() {
        lemon::SmartDigraph g;
        lemon::SmartDigraph::Node s = g.addNode(), t = g.addNode();
        g.addArc(s, t);
        lemon::SmartDigraph::ArcMap<int> cost(g, 2), cap(g, 5);
        lemon::SmartDigraph::NodeMap<int> supply(g, 0);
        supply[s] = 3;
        supply[t] = -3;
        lemon::NetworkSimplex<lemon::SmartDigraph> ns(g);
        ns.costMap(cost).upperMap(cap).supplyMap(supply);
        return ns.run() == ns.OPTIMAL && ns.totalCost() == 6 ? 0 : 1;
      }
    CPP
    system ENV.cxx, "test.cpp", "-std=c++11", "-I#{include}", "-L#{lib}",
           "-lemon", "-o", "test"
    system "./test"
  end
end
