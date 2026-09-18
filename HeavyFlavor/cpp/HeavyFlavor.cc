// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/HeavyFlavor/cpp/HeavyFlavor.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::heavyflavor {

  using tga::decayMode;
  using tga::firstChildWithPdgId;
  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    for (auto const& branch : truth::branchesAtLevel(graph, truth::Level::BHadrons)) {
      const truth::Particle hadron = branch.root();
      const auto production = hadron.productionVertices();
      const auto decay = hadron.decayVertices();
      std::string flight = "n/a";
      if (!production.empty() && !decay.empty()) {
        const auto& x0 = production.front().data().position;
        const auto& x1 = decay.front().data().position;
        flight = fixed(std::sqrt((x1.x() - x0.x()) * (x1.x() - x0.x()) + (x1.y() - x0.y()) * (x1.y() - x0.y()) +
                                 (x1.z() - x0.z()) * (x1.z() - x0.z())),
                       3) +
                 " cm";
      }
      std::size_t charm = 0;
      for (auto const& member : branch.members()) {
        charm += member.data().isAtLevel(truth::LevelFlag::CHadrons) ? 1 : 0;
      }
      out << "heavyflavor: " << hadron.pdgId() << " pt " << fixed(hadron.momentum().pt()) << " GeV, flight " << flight
          << ", " << charm << " charm hadron" << (charm == 1 ? "" : "s") << " below\n";
    }
  }

}  // namespace tga::heavyflavor
