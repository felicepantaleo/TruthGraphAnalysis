// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Diboson/cpp/Diboson.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::diboson {

  using tga::decayMode;
  using tga::firstChildWithPdgId;
  using tga::fixed;

  void run(truth::Graph const& graph, std::ostream& out) {
    std::vector<truth::Particle> bosons;
    for (auto const& root : graph.signalParticles()) {
      if (truth::isWeakBoson(root.pdgId())) {
        bosons.push_back(root);
      }
    }
    if (bosons.size() >= 2) {
      out << "diboson: m(VV) " << fixed((bosons[0].momentum() + bosons[1].momentum()).mass()) << " GeV\n";
    }
    for (auto const& boson : bosons) {
      out << "  " << boson.pdgId() << " pt " << fixed(boson.momentum().pt()) << " GeV, decay " << decayMode(boson)
          << "\n";
    }
  }

}  // namespace tga::diboson
