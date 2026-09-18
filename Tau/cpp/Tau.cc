// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Tau/cpp/Tau.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <string>

#include "DataFormats/Math/interface/LorentzVector.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::tau {

  using tga::fixed;

  namespace {

    // What one tau decayed into, from the generator record.
    struct Content {
      int prongs = 0;  // charged pions and kaons
      int pi0s = 0;
      int neutralKaons = 0;  // K0S and K0L: neutral, so not prongs
      int photons = 0;       // radiated in the decay, not from a pi0
      int electrons = 0;
      int muons = 0;
      int others = 0;
      math::XYZTLorentzVectorD visible;
    };

    // Follows the generator decay chain down to what leaves it. It goes through an
    // intermediate resonance such as the rho or the a1, stops at a pi0 and at a neutral
    // kaon, which the decay mode counts as such, and stops at a particle the generator did
    // not decay. Only generator decay vertices are followed, so an interaction in the
    // detector is never taken for a decay product.
    void collect(truth::Particle const& particle, Content& content) {
      const int32_t pdgId = particle.pdgId();
      const int32_t a = std::abs(pdgId);
      if (truth::isInvisible(pdgId)) {
        return;
      }
      if (a == 111 || a == 130 || a == 310) {
        ++(a == 111 ? content.pi0s : content.neutralKaons);
        content.visible += particle.momentum();
        return;
      }
      bool decayed = false;
      for (auto const& vertex : particle.decayVertices()) {
        if (!vertex.hasGen()) {
          continue;
        }
        decayed = true;
        for (auto const& child : vertex.outgoingParticles()) {
          if (child != particle) {
            collect(child, content);
          }
        }
      }
      if (decayed) {
        return;
      }
      if (a == 211 || a == 321) {
        ++content.prongs;
      } else if (a == 22) {
        ++content.photons;
      } else if (a == 11) {
        ++content.electrons;
      } else if (a == 13) {
        ++content.muons;
      } else {
        ++content.others;
      }
      content.visible += particle.momentum();
    }

    std::string modeName(Content const& content, truth::TauDecay decay) {
      if (decay == truth::TauDecay::Leptonic) {
        return content.electrons > 0 ? "electron" : "muon";
      }
      std::string name = std::to_string(content.prongs) + "prong" + std::to_string(content.pi0s) + "pi0";
      if (content.neutralKaons > 0) {
        name += std::to_string(content.neutralKaons) + "K0";
      }
      return name;
    }

    // reco::PFTau::hadronicDecayMode: 5 * (prongs - 1) + pi0s with the pi0 count capped at
    // 4, kRareDecayMode (15) outside one to three prongs, kNull (-1) for a leptonic decay,
    // which tau reconstruction does not classify.
    int recoDecayMode(Content const& content, truth::TauDecay decay) {
      if (decay != truth::TauDecay::Hadronic) {
        return -1;
      }
      if (content.prongs < 1 || content.prongs > 3) {
        return 15;
      }
      return 5 * (content.prongs - 1) + std::min(content.pi0s, 4);
    }

  }  // namespace

  void run(truth::Graph const& graph, std::ostream& out) {
    std::map<std::string, int> modes;
    // Both tau levels at once: each is the last copy of a physical tau, and a graph needs
    // no selection preset to carry them.
    const auto taus =
        truth::particlesAtLevels(graph, {truth::Level::TauVisibleHadronic, truth::Level::TauVisibleLeptonic});
    for (auto const& tau : taus) {
      const truth::TauDecay decay = truth::tauDecay(graph, tau.id());
      Content content;
      collect(tau, content);
      const std::string name = modeName(content, decay);
      ++modes[name];
      out << "tau " << tau.pdgId() << " E " << fixed(tau.momentum().energy()) << " GeV: " << name << ", reco mode "
          << recoDecayMode(content, decay) << ", " << content.prongs << " prongs " << content.pi0s << " pi0 "
          << content.photons << " photons";
      // A stable product outside the species above would otherwise pass unseen.
      if (content.others > 0) {
        out << " " << content.others << " others";
      }
      out << ", visible energy " << fixed(content.visible.energy() / tau.momentum().energy()) << "\n";
    }
    for (auto const& [name, count] : modes) {
      out << "mode " << name << ": " << count << "\n";
    }
  }

}  // namespace tga::tau
