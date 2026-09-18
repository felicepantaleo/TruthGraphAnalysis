// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Top/cpp/Top.h"

#include <cmath>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::top {

  using tga::decayMode;
  using tga::firstChildWithPdgId;

  void run(truth::Graph const& graph, std::ostream& out) {
    int tops = 0;
    int classified = 0;
    int leptonic = 0;
    for (auto const& t : graph.signalParticles()) {
      if (std::abs(t.pdgId()) != 6) {
        continue;
      }
      ++tops;
      const auto b = firstChildWithPdgId(t, {5});
      const auto w = firstChildWithPdgId(t, {24});
      const std::string mode = w ? decayMode(*w) : "none";
      classified += (mode == "none") ? 0 : 1;
      leptonic += (mode == "leptonic") ? 1 : 0;
      out << "top " << t.pdgId() << ": b " << (b ? "yes" : "no") << ", W " << mode << ", " << t.descendants().size()
          << " descendants\n";
    }
    // Anything but two tops with both W decays found is not a ttbar event this can name.
    const char* eventClass = (tops != 2 || classified != 2) ? "unclassified"
                             : leptonic == 0                ? "all hadronic"
                             : leptonic == 1                ? "semileptonic"
                                                            : "dilepton";
    out << "top event class: " << eventClass << "\n";
  }

}  // namespace tga::top
