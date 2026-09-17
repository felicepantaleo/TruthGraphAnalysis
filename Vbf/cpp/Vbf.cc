// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Vbf/cpp/Vbf.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::vbf {

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
    const auto higgs = signalRoots(graph);
    std::vector<truth::Particle> tagging;
    for (auto const& parton : membersOf(graph, truth::Level::PartonJets)) {
      if (parton.data().isSignal()) {
        tagging.push_back(parton);
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
