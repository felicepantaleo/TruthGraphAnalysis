// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// What does the whole event hold, interaction by interaction, and how large is the reconstructable final state of the signal and of the pile-up?

#ifndef TruthGraphAnalysis_Full_Full_h
#define TruthGraphAnalysis_Full_Full_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::full {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::full

#endif
