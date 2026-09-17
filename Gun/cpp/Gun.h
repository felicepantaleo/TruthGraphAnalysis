// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// Per gun particle: how many reconstructable products does it have, and how many of its descendants reach the calorimeter?

#ifndef TruthGraphAnalysis_Gun_Gun_h
#define TruthGraphAnalysis_Gun_Gun_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::gun {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::gun

#endif
