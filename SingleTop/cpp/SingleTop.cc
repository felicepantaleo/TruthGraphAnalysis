// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/SingleTop/cpp/SingleTop.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::singletop {

  using tga::decayMode;
  using tga::firstChildWithPdgId;
  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    for (auto const& t : graph.signalParticles()) {
      for (auto const& partner : t.productionSiblings()) {
        out << "singletop: top with partner " << partner.pdgId() << " pt " << fixed(partner.momentum().pt())
            << " GeV\n";
      }
    }
  }

}  // namespace tga::singletop
