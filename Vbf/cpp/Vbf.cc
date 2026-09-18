// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Vbf/cpp/Vbf.h"

#include <cmath>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::vbf {

  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    const auto higgs = graph.signalParticles();
    // The tagging quarks are the partons produced with the Higgs. The parton-jet level is
    // not the answer: it also holds the quarks a hadronic Higgs decay makes.
    std::vector<truth::Particle> tagging;
    if (!higgs.empty()) {
      for (auto const& sibling : higgs.front().productionSiblings()) {
        if (truth::isParton(sibling.pdgId())) {
          tagging.push_back(sibling);
        }
      }
    }
    if (tagging.size() < 2) {
      out << "vbf: " << higgs.size() << " Higgs, " << tagging.size() << " tagging partons\n";
      return;
    }
    std::stable_sort(tagging.begin(), tagging.end(), [](auto const& a, auto const& b) {
      return a.momentum().pt() > b.momentum().pt();
    });
    const auto dijet = tagging[0].momentum() + tagging[1].momentum();
    out << "vbf: " << higgs.size() << " Higgs, tagging partons " << tagging[0].pdgId() << " " << tagging[1].pdgId()
        << ": m(jj) " << fixed(dijet.mass()) << " GeV, |delta eta| "
        << fixed(std::abs(tagging[0].momentum().eta() - tagging[1].momentum().eta())) << "\n";
  }

}  // namespace tga::vbf
