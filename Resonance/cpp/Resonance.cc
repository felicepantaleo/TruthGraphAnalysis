// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Resonance/cpp/Resonance.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::resonance {

  using tga::decayMode;
  using tga::firstChildWithPdgId;
  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    for (auto const& z : graph.signalParticles()) {
      std::vector<truth::Particle> legs;
      for (auto const& child : z.lastCopy().children()) {
        if (truth::isLepton(child.pdgId())) {
          legs.push_back(child);
        }
      }
      if (legs.size() != 2) {
        out << "resonance " << z.pdgId() << ": decay mode " << decayMode(z) << "\n";
        continue;
      }
      const auto dilepton = legs[0].momentum() + legs[1].momentum();
      out << "resonance " << z.pdgId() << " -> " << legs[0].pdgId() << " " << legs[1].pdgId() << ": m(ll) "
          << fixed(dilepton.mass()) << " GeV, generator mass " << fixed(z.momentum().mass()) << " GeV\n";
    }
  }

}  // namespace tga::resonance
