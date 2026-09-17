// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// A truth::Graph read back from the JSON that TruthLogicalGraphDumper writes with
// jsonFile set. It carries what the examples read: species, status, levels, momenta,
// interaction ids, roles, reasons and positions, and the full adjacency.

#ifndef TruthGraphAnalysis_Common_GraphFromJson_h
#define TruthGraphAnalysis_Common_GraphFromJson_h

#include <string>

#include "SimDataFormats/TruthInfo/interface/Graph.h"

namespace tga {

  [[nodiscard]] truth::Graph graphFromJson(std::string const& path);

}  // namespace tga

#endif
