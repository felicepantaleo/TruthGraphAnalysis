// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/SingleTop/cpp/SingleTop.h"

#include <cmath>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::singletop {

  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    for (auto const& t : graph.signalParticles()) {
      for (auto const& partner : t.productionSiblings()) {
        const int32_t a = std::abs(partner.pdgId());
        // A top partner is ttbar, not single top; the top is a parton by its pdgId, so it is
        // named before the parton test.
        const char* kind = a == 24              ? "associated W"
                           : a == 6             ? "top"
                           : a == 5             ? "b"
                           : truth::isParton(a) ? "recoil quark"
                                                : "other";
        out << "singletop: top with " << kind << " " << partner.pdgId() << " pt " << fixed(partner.momentum().pt())
            << " GeV\n";
      }
    }
  }

}  // namespace tga::singletop
