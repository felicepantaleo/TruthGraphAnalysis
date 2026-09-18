// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Resonance/cpp/Resonance.h"

#include <cmath>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::resonance {

  using tga::decayMode;
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
      // The decay legs carry the Z four-vector exactly. The last copy of each lepton is what
      // is left after final-state radiation, which is what a detector measures.
      const auto fromDecay = legs[0].momentum() + legs[1].momentum();
      const auto afterRadiation = legs[0].lastCopy().momentum() + legs[1].lastCopy().momentum();
      out << "resonance " << z.pdgId() << " -> " << legs[0].pdgId() << " " << legs[1].pdgId() << ": generator mass "
          << fixed(z.momentum().mass()) << " GeV, m(ll) " << fixed(fromDecay.mass()) << " GeV from the decay, "
          << fixed(afterRadiation.mass()) << " GeV after FSR\n";
    }
  }

}  // namespace tga::resonance
