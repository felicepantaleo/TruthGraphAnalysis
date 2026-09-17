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
  using tga::isLepton;
  using tga::isWeakBoson;
  using tga::lastCopy;
  using tga::membersOf;
  using tga::productionSiblings;
  using tga::signalRoots;

  void run(truth::Graph const& graph, std::ostream& out) {
    for (auto const& t : signalRoots(graph)) {
      for (auto const& partner : productionSiblings(t)) {
        out << "singletop: top with partner " << partner.pdgId() << " pt " << fixed(partner.momentum().pt())
            << " GeV\n";
      }
    }
  }

}  // namespace tga::singletop
