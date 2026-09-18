// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Ggf/cpp/Ggf.h"

#include <cmath>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::ggf {

  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    for (auto const& higgs : graph.signalParticles()) {
      const truth::Branch branch(&graph, higgs.id());
      std::size_t products = 0;
      double visible = 0.;
      bool coloured = false;
      for (auto const& member : branch.members()) {
        coloured = coloured || truth::isParton(member.pdgId());
        if (!member.data().isAtLevel(truth::LevelFlag::ReconstructableFromSignal)) {
          continue;
        }
        ++products;
        visible += truth::isInvisible(member.pdgId()) ? 0. : member.momentum().energy();
      }
      // A coloured decay hadronizes with partons from outside the Higgs, so its products carry
      // their energy too and the fraction would exceed 1.
      out << "ggf: Higgs E " << fixed(higgs.momentum().energy()) << " GeV -> " << products
          << " reconstructable products, visible fraction "
          << (coloured ? std::string("n/a, coloured decay") : fixed(visible / higgs.momentum().energy(), 3)) << "\n";
    }
  }

}  // namespace tga::ggf
