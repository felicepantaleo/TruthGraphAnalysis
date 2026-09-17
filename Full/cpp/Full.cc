// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Full/cpp/Full.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "PhysicsTools/TruthInfo/interface/Branch.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::full {

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
    std::map<uint64_t, std::size_t> particlesOf;
    for (auto const& particle : graph.particles()) {
      ++particlesOf[particle.eventId];
    }
    for (auto const& [eventId, count] : particlesOf) {
      out << "full: interaction bx " << truth::bunchCrossingOf(eventId) << " index " << truth::eventIndexOf(eventId)
          << ": " << count << " particles\n";
    }
    for (const bool signal : {true, false}) {
      std::size_t objects = 0;
      std::size_t withoutMomentum = 0;
      double energy = 0.;
      // The members the producer stamped, as the python twin reads them.
      for (auto const& member : graph.particles()) {
        if (!member.isAtLevel(truth::LevelFlag::ReconstructableFinalState) || member.isSignal() != signal) {
          continue;
        }
        ++objects;
        energy += member.momentum.energy();
        withoutMomentum += member.hasMomentum() ? 0 : 1;
      }
      out << "full: " << (signal ? "signal" : "pileup") << " reconstructable final state " << objects << " objects, "
          << fixed(energy, 1) << " GeV, " << withoutMomentum << " without momentum\n";
    }
  }

}  // namespace tga::full
