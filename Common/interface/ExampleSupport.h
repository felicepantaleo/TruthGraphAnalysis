// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// What every C++ example needs and the interface does not provide as such: the signal
// roots, the members of a level as views, a decay mode, the production siblings, and one
// number format so the python twin prints the same line.

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

  inline bool isLepton(int32_t pdgId) {
    const int32_t a = std::abs(pdgId);
    return a == 11 || a == 13 || a == 15;
  }

  inline bool isWeakBoson(int32_t pdgId) {
    const int32_t a = std::abs(pdgId);
    return a == 23 || a == 24;
  }

  inline std::vector<truth::Particle> membersOf(truth::Graph const& graph, truth::Level level) {
    std::vector<truth::Particle> members;
    for (const uint32_t id : truth::levelAntichain(graph, level)) {
      members.emplace_back(&graph, id);
    }
    return members;
  }

  // The particles the selection flagged as the signal, in id order.
  inline std::vector<truth::Particle> signalRoots(truth::Graph const& graph) {
    std::vector<truth::Particle> roots;
    for (uint32_t id = 0; id < graph.nParticles(); ++id) {
      if (graph.particles()[id].isAtLevel(truth::LevelFlag::Signal)) {
        roots.emplace_back(&graph, id);
      }
    }
    return roots;
  }

  // The copy of a radiating chain whose children are the decay products.
  inline truth::Particle lastCopy(truth::Particle const& particle) {
    return truth::Particle(particle.graph(), truth::lastCopyOf(*particle.graph(), particle.id()));
  }

  // The first child whose species is in the list, read from the last copy, without
  // building the child vector.
  inline std::optional<truth::Particle> firstChildWithPdgId(truth::Particle const& copy,
                                                            std::vector<int32_t> const& pdgIds) {
    const truth::Particle particle = lastCopy(copy);
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
    const truth::Particle boson = lastCopy(copy);
    std::string mode = "none";
    boson.forEachChildId([&](uint32_t child) {
      const int32_t pdgId = std::abs(boson.graph()->particles()[child].pdgId);
      if (isLepton(pdgId)) {
        mode = "leptonic";
      } else if (pdgId <= 6 && mode == "none") {
        mode = "hadronic";
      }
    });
    return mode;
  }

  // The other particles produced where this one was.
  inline std::vector<truth::Particle> productionSiblings(truth::Particle const& particle) {
    std::vector<truth::Particle> siblings;
    for (auto const& vertex : particle.productionVertices()) {
      for (auto const& out : vertex.outgoingParticles()) {
        if (out != particle) {
          siblings.push_back(out);
        }
      }
    }
    return siblings;
  }

}  // namespace tga

#endif
