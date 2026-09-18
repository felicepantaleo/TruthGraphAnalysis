// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// The two things the examples need that are analysis choices rather than graph
// navigation: how a boson decay is named, and one number format so the python twin
// prints the same line. Everything else comes from PhysicsTools/TruthInfo.

#ifndef TruthGraphAnalysis_Common_ExampleSupport_h
#define TruthGraphAnalysis_Common_ExampleSupport_h

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/Particle.h"

namespace tga {

  inline std::string fixed(double value, int decimals = 2) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(decimals) << value;
    return out.str();
  }

  // The first child of any of these species, either charge, read from the last copy.
  // Particle::firstChildWithPdgId matches one signed id, which is the graph's own
  // question; matching a list either way round is this analysis asking.
  inline std::optional<truth::Particle> firstChildWithPdgId(truth::Particle const& copy,
                                                            std::vector<int32_t> const& pdgIds) {
    const truth::Particle particle = copy.lastCopy();
    std::optional<truth::Particle> found;
    particle.forEachChildId([&](uint32_t child) {
      if (found) {
        return;
      }
      const int32_t pdgId = std::abs(particle.graph()->particles()[child].pdgId);
      if (std::find(pdgIds.begin(), pdgIds.end(), pdgId) != pdgIds.end()) {
        found.emplace(particle.graph(), child);
      }
    });
    return found;
  }

  // leptonic, hadronic or none, from the children of the last copy of a W or a Z. A tau
  // counts as a lepton here, so W -> tau nu is leptonic whatever the tau does next.
  inline std::string decayMode(truth::Particle const& copy) {
    const truth::Particle boson = copy.lastCopy();
    std::string mode = "none";
    boson.forEachChildId([&](uint32_t child) {
      const int32_t pdgId = boson.graph()->particles()[child].pdgId;
      if (truth::isLepton(pdgId)) {
        mode = "leptonic";
      } else if (truth::isParton(pdgId) && mode == "none") {
        mode = "hadronic";
      }
    });
    return mode;
  }

}  // namespace tga

#endif
