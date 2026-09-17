// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// What are the dijet mass and the rapidity gap of the two quarks that recoil against the Higgs?

#ifndef TruthGraphAnalysis_Vbf_Vbf_h
#define TruthGraphAnalysis_Vbf_Vbf_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::vbf {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::vbf

#endif
