// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// How did each tau decay: how many charged hadrons and neutral pions, which decay mode
// tau reconstruction would call it, and how much of the tau energy is visible?

#ifndef TruthGraphAnalysis_Tau_Tau_h
#define TruthGraphAnalysis_Tau_Tau_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::tau {

  // Prints one line per physical tau, then the count of each decay mode. The python twin
  // prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::tau

#endif
