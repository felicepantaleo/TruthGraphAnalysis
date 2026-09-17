// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// How far does each b hadron fly before it decays, and how many charm hadrons does it leave below it?

#ifndef TruthGraphAnalysis_HeavyFlavor_HeavyFlavor_h
#define TruthGraphAnalysis_HeavyFlavor_HeavyFlavor_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::heavyflavor {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::heavyflavor

#endif
