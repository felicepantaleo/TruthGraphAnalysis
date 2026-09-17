// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// Which two leptons does the Z decay to, and does their invariant mass reproduce the generator mass of the Z?

#ifndef TruthGraphAnalysis_Resonance_Resonance_h
#define TruthGraphAnalysis_Resonance_Resonance_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::resonance {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::resonance

#endif
