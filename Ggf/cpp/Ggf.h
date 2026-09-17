// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// What does the detector see of the Higgs: how many reconstructable products, and which fraction of its energy is visible?

#ifndef TruthGraphAnalysis_Ggf_Ggf_h
#define TruthGraphAnalysis_Ggf_Ggf_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::ggf {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::ggf

#endif
