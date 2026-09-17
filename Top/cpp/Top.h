// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// For each top: is there a b, how did the W decay, how large is its subgraph; and what class is the event, all hadronic, semileptonic or dilepton? A W to tau nu counts as leptonic.

#ifndef TruthGraphAnalysis_Top_Top_h
#define TruthGraphAnalysis_Top_Top_h

#include <ostream>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga::top {

  // Prints one line per object of interest. The python twin prints the same lines.
  void run(truth::Graph const& graph, std::ostream& out);

}  // namespace tga::top

#endif
