// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// In a boosted tau or top the footprints of the decay products overlap. Which detector
// cells do the products share, and does each product keep its label: from the hits, and
// from a Delta R match?

#ifndef TruthGraphAnalysis_Boosted_Boosted_h
#define TruthGraphAnalysis_Boosted_Boosted_h

#include <cstdint>
#include <ostream>
#include <vector>

#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/LogicalGraphHitIndex.h"

namespace tga::boosted {

  // One decay product: its species, its axis and its hits. A hit is the energy in one
  // detector cell, and a cell can repeat in the list.
  struct Product {
    int32_t pdgId = 0;
    double eta = 0.;
    double phi = 0.;
    std::vector<truth::LogicalGraphHitIndex::Hit> hits;
  };

  // The cone of the Delta R match: the radius of an AK4 jet.
  inline constexpr double kCone = 0.4;

  // The hit logic, with no graph. Prints one line for the parent, one line per product and
  // the count of the labels kept. The python twin prints the same lines.
  //   shared cell      a cell with energy from two or more products
  //   purity           the energy of the product over the energy of all products in its cells
  //   hit label        kept when the purity is above 0.5
  //   Delta R label    kept when no other product axis is inside the cone
  void report(int32_t parentPdgId, std::vector<Product> const& products, double cone, std::ostream& out);

  enum class Parent { Tau, Top };

  // Calls report for each tau of the two tau levels, or for the last copy of each top. The
  // products are the visible generator decay products of the parent, and the hits of a
  // product are its calorimeter hits and those of its descendants.
  void run(truth::Graph const& graph,
           truth::LogicalGraphHitIndex const& hitIndex,
           Parent parent,
           double cone,
           std::ostream& out);

}  // namespace tga::boosted

#endif
