// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// What is the mass of the boson pair, and how did each boson decay?

#ifndef TruthGraphAnalysis_Diboson_Diboson_h
#define TruthGraphAnalysis_Diboson_Diboson_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::diboson {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::diboson

#endif
