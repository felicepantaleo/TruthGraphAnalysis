// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// Which vector boson was produced together with the Higgs, and how did it decay?

#ifndef TruthGraphAnalysis_Vh_Vh_h
#define TruthGraphAnalysis_Vh_Vh_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::vh {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::vh

#endif
