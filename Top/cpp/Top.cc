// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Top/cpp/Top.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::top {

  using tga::decayMode;
  using tga::firstChildWithPdgId;
  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    int leptonic = 0;
    for (auto const& t : graph.signalParticles()) {
      const auto b = firstChildWithPdgId(t, {5});
      const auto w = firstChildWithPdgId(t, {24});
      const std::string mode = w ? decayMode(*w) : "none";
      leptonic += (mode == "leptonic") ? 1 : 0;
      out << "top " << t.pdgId() << ": b " << (b ? "yes" : "no") << ", W " << mode << ", " << t.descendants().size()
          << " descendants\n";
    }
    const char* eventClass = leptonic == 0 ? "all hadronic" : leptonic == 1 ? "semileptonic" : "dilepton";
    out << "top event class: " << eventClass << "\n";
  }

}  // namespace tga::top
