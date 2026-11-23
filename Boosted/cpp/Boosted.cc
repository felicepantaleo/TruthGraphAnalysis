// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Boosted/cpp/Boosted.h"

#include <cstddef>
#include <cstdlib>
#include <map>
#include <utility>

#include "DataFormats/Math/interface/deltaR.h"
#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"
#include "SimDataFormats/TruthInfo/interface/Particle.h"
#include "SimDataFormats/TruthInfo/interface/Vertex.h"
#include "TruthGraphAnalysis/Common/interface/ExampleSupport.h"

namespace tga::boosted {

  using tga::fixed;

  namespace {

    constexpr double kMinimumPurity = 0.5;

    // What a product, or the parent as a whole, left in the cells.
    struct Footprint {
      int cells = 0;
      double energy = 0.;
      int sharedCells = 0;
      double sharedEnergy = 0.;
      double cellEnergy = 0.;  // of all products, in the cells of this product
    };

    // The visible decay products: what leaves the generator decay vertices of the last
    // copy. A top gives a b and a W. The W stays one product: its two quarks hadronize
    // together, so the graph gives the same hadrons to both.
    std::vector<truth::Particle> decayProducts(truth::Particle const& particle) {
      std::vector<truth::Particle> products;
      const truth::Particle last = particle.lastCopy();
      for (auto const& vertex : last.decayVertices()) {
        if (!vertex.hasGen()) {
          continue;
        }
        for (auto const& child : vertex.outgoingParticles()) {
          if (child != last && !truth::isInvisible(child.pdgId())) {
            products.push_back(child);
          }
        }
      }
      return products;
    }

    std::vector<truth::Particle> parents(truth::Graph const& graph, Parent parent) {
      if (parent == Parent::Tau) {
        return truth::particlesAtLevels(graph, {truth::Level::TauVisibleHadronic, truth::Level::TauVisibleLeptonic});
      }
      // A top is a member of no level unless the graph has the top preset, so take the
      // last copy of each top.
      std::vector<truth::Particle> tops;
      for (uint32_t id = 0; id < graph.particles().size(); ++id) {
        const truth::Particle particle(&graph, id);
        if (std::abs(particle.pdgId()) == 6 && particle.lastCopy() == particle) {
          tops.push_back(particle);
        }
      }
      return tops;
    }

  }  // namespace

  void report(int32_t parentPdgId, std::vector<Product> const& products, double cone, std::ostream& out) {
    const std::size_t n = products.size();
    // The energy of each product in each cell, the cells in DetId order. A hit with no
    // energy is no hit.
    std::map<uint32_t, std::vector<double>> cells;
    for (std::size_t i = 0; i < n; ++i) {
      for (auto const& hit : products[i].hits) {
        if (hit.energy > 0.f) {
          cells.try_emplace(hit.detId, n, 0.).first->second[i] += hit.energy;
        }
      }
    }

    Footprint all;
    std::vector<Footprint> own(n);
    for (auto const& [detId, energies] : cells) {
      double cellEnergy = 0.;
      int owners = 0;
      for (const double energy : energies) {
        cellEnergy += energy;
        owners += energy > 0. ? 1 : 0;
      }
      const bool shared = owners > 1;
      ++all.cells;
      all.energy += cellEnergy;
      if (shared) {
        ++all.sharedCells;
        all.sharedEnergy += cellEnergy;
      }
      for (std::size_t i = 0; i < n; ++i) {
        if (energies[i] <= 0.) {
          continue;
        }
        ++own[i].cells;
        own[i].energy += energies[i];
        own[i].cellEnergy += cellEnergy;
        if (shared) {
          ++own[i].sharedCells;
          own[i].sharedEnergy += energies[i];
        }
      }
    }

    out << "parent " << parentPdgId << ": " << n << " products, " << all.cells << " cells " << fixed(all.energy)
        << " GeV, " << all.sharedCells << " shared cells " << fixed(all.sharedEnergy) << " GeV, shared energy fraction "
        << fixed(all.energy > 0. ? all.sharedEnergy / all.energy : 0.) << "\n";

    int keptByDeltaR = 0;
    int keptByHits = 0;
    for (std::size_t i = 0; i < n; ++i) {
      const double purity = own[i].cellEnergy > 0. ? own[i].energy / own[i].cellEnergy : 0.;
      const bool hitLabel = purity > kMinimumPurity;

      // The nearest other product: a Delta R match cannot tell two axes in one cone apart.
      std::size_t nearest = n;
      double nearestDeltaR = 0.;
      for (std::size_t j = 0; j < n; ++j) {
        if (j == i) {
          continue;
        }
        const double dR = reco::deltaR(products[i].eta, products[i].phi, products[j].eta, products[j].phi);
        if (nearest == n || dR < nearestDeltaR) {
          nearest = j;
          nearestDeltaR = dR;
        }
      }
      const bool deltaRLabel = nearest == n || nearestDeltaR >= cone;
      keptByDeltaR += deltaRLabel ? 1 : 0;
      keptByHits += hitLabel ? 1 : 0;

      out << "  product " << products[i].pdgId << ": " << own[i].cells << " cells " << fixed(own[i].energy) << " GeV, "
          << own[i].sharedCells << " shared " << fixed(own[i].sharedEnergy) << " GeV, purity " << fixed(purity) << ", ";
      if (nearest == n) {
        out << "no other product";
      } else {
        out << "nearest product " << products[nearest].pdgId << " at dR " << fixed(nearestDeltaR);
      }
      out << ", Delta R label " << (deltaRLabel ? "kept" : "lost") << ", hit label " << (hitLabel ? "kept" : "lost")
          << ", same label " << (deltaRLabel == hitLabel ? "yes" : "no") << "\n";
    }
    out << "  labels kept: Delta R " << keptByDeltaR << " of " << n << ", hits " << keptByHits << " of " << n << "\n";
  }

  void run(truth::Graph const& graph,
           truth::LogicalGraphHitIndex const& hitIndex,
           Parent parent,
           double cone,
           std::ostream& out) {
    for (auto const& particle : parents(graph, parent)) {
      std::vector<Product> products;
      for (auto const& child : decayProducts(particle)) {
        Product product{child.pdgId(), child.momentum().eta(), child.momentum().phi(), {}};
        // A pi0, a quark or a W has no hit of its own: its footprint is in its subgraph.
        hitIndex.appendSubgraphHits(truth::HitChannel::Calo, child.id(), product.hits);
        products.push_back(std::move(product));
      }
      report(particle.pdgId(), products, cone, out);
    }
  }

}  // namespace tga::boosted
