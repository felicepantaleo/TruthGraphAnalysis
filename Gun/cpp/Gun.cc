// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Gun/cpp/Gun.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::gun {

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
    for (auto const& seed : signalRoots(graph)) {
      const truth::Branch branch(&graph, seed.id());
      std::size_t products = 0;
      std::size_t atCalo = 0;
      for (auto const& member : branch.members()) {
        products += member.data().isAtLevel(truth::LevelFlag::ReconstructableFromSignal) ? 1 : 0;
        atCalo += member.data().isAtLevel(truth::LevelFlag::CaloBoundary) ? 1 : 0;
      }
      out << "gun " << seed.pdgId() << " E " << fixed(seed.momentum().energy()) << " GeV: " << products
          << " reconstructable products, " << atCalo << " descendants reach the calorimeter\n";
    }
  }

}  // namespace tga::gun
