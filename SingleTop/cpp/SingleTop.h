// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// What was produced together with the top: the recoil quark, the associated W, or the b?

#ifndef TruthGraphAnalysis_SingleTop_SingleTop_h
#define TruthGraphAnalysis_SingleTop_SingleTop_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::singletop {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::singletop

#endif
