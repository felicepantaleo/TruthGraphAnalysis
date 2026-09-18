// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Vh/cpp/Vh.h"

#include <cmath>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::vh {

  using tga::decayMode;
  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    for (auto const& higgs : graph.signalParticles()) {
      for (auto const& sibling : higgs.productionSiblings()) {
        if (truth::isWeakBoson(sibling.pdgId())) {
          out << "vh: Higgs with " << sibling.pdgId() << " pt " << fixed(sibling.momentum().pt())
              << " GeV, boson decay " << decayMode(sibling) << "\n";
        }
      }
    }
  }

}  // namespace tga::vh
