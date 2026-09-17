// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Vh/cpp/Vh.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::vh {

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
    for (auto const& higgs : signalRoots(graph)) {
      for (auto const& sibling : productionSiblings(higgs)) {
        if (isWeakBoson(sibling.pdgId())) {
          out << "vh: Higgs with " << sibling.pdgId() << " pt " << fixed(sibling.momentum().pt())
              << " GeV, boson decay " << decayMode(sibling) << "\n";
        }
      }
    }
  }

}  // namespace tga::vh
