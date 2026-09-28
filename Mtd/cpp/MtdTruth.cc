// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Mtd/cpp/MtdTruth.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

#include "DataFormats/ForwardDetId/interface/ETLDetId.h"
#include "DataFormats/ForwardDetId/interface/MTDDetId.h"
#include "SimDataFormats/TruthInfo/interface/Particle.h"

namespace tga::mtd {

  std::optional<math::XYZTLorentzVectorD> productionPoint(truth::Graph const& graph, uint32_t particleId) {
    const auto vertices = graph.productionVertices(particleId);
    if (vertices.empty())
      return std::nullopt;
    return graph.vertices()[vertices.front()].position;
  }

  bool passesSelection(truth::Graph const& graph, uint32_t particleId, TruthSelection const& cuts) {
    if (truth::Particle(&graph, particleId).charge() == 0.)
      return false;
    auto const& p4 = graph.particles()[particleId].momentum;
    if (std::abs(p4.eta()) >= cuts.etaMax || p4.pt() <= cuts.ptMin)
      return false;
    const auto point = productionPoint(graph, particleId);
    return point && point->Rho() < cuts.productionRhoMax && std::abs(point->Z()) < cuts.productionZMax;
  }

  bool isGeneratorFinalState(truth::Graph const& graph, uint32_t particleId) {
    return graph.particles()[particleId].status == 1;
  }

  bool Deposit::isBarrel() const { return MTDDetId(module).mtdSubDetector() == MTDDetId::BTL; }

  int Deposit::etlDisc() const { return isBarrel() ? 0 : ETLDetId(module).nDisc(); }

  bool Deposit::sharesCell(uint32_t otherModule, std::vector<uint32_t> const& sortedCells) const {
    if (otherModule != module)
      return false;
    auto a = cells.begin();
    auto b = sortedCells.begin();
    while (a != cells.end() && b != sortedCells.end()) {
      if (*a == *b)
        return true;
      if (*a < *b)
        ++a;
      else
        ++b;
    }
    return false;
  }

  std::vector<Deposit> deposits(std::span<const truth::LogicalGraphHitIndex::Hit> hits, std::span<const float> times) {
    std::vector<Deposit> result;
    if (hits.size() != times.size())
      return result;
    std::map<std::pair<uint32_t, uint32_t>, Deposit> byKey;
    for (std::size_t i = 0; i < hits.size(); ++i) {
      auto const& hit = hits[i];
      const uint32_t category = categoryOf(hit.recHitIndex);
      auto& deposit = byKey[{hit.detId, category}];
      deposit.module = hit.detId;
      deposit.category = category;
      deposit.energy += hit.energy;
      deposit.time += static_cast<double>(hit.energy) * times[i];
      deposit.cells.push_back(cellOf(hit.recHitIndex));
    }
    result.reserve(byKey.size());
    for (auto& [key, deposit] : byKey) {
      if (deposit.energy > 0.)
        deposit.time /= deposit.energy;
      std::sort(deposit.cells.begin(), deposit.cells.end());
      result.push_back(std::move(deposit));
    }
    return result;
  }

  bool matches(Deposit const& deposit,
               uint32_t module,
               std::vector<uint32_t> const& sortedCells,
               double recoEnergyGeV,
               double recoTime,
               double recoTimeError,
               ClusterMatchCuts const& cuts) {
    if (!deposit.sharesCell(module, sortedCells) || recoTimeError <= 0.)
      return false;
    if (std::abs(recoTime - deposit.time) / recoTimeError >= cuts.timePullMax)
      return false;
    return !deposit.isBarrel() || (deposit.energy > 0. && recoEnergyGeV / deposit.energy < cuts.energyRatioMax);
  }

}  // namespace tga::mtd
