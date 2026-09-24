// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// The truth side of the MTD track validation, read from the truth graph: which particle
// a track comes from, whether that particle passes the TrackingParticle selection of
// MtdTracksValidation, and what it left in the MTD.

#ifndef TruthGraphAnalysis_Mtd_MtdTruth_h
#define TruthGraphAnalysis_Mtd_MtdTruth_h

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "DataFormats/Math/interface/LorentzVector.h"
#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/LogicalGraphHitIndex.h"

namespace tga::mtd {

  // The cuts MtdTracksValidation::trkTPSelAll applies to a TrackingParticle.
  struct TruthSelection {
    double etaMax = 4.;
    double ptMin = 0.7;              // GeV
    double productionRhoMax = 110.;  // cm
    double productionZMax = 290.;    // cm
  };

  // The production point of a particle in (cm, ns). Empty for a particle with no
  // production vertex, which is a beam particle.
  [[nodiscard]] std::optional<math::XYZTLorentzVectorD> productionPoint(truth::Graph const& graph, uint32_t particleId);

  // Charged, inside the eta and pt cuts, and produced inside the MTD volume.
  [[nodiscard]] bool passesSelection(truth::Graph const& graph, uint32_t particleId, TruthSelection const& cuts);

  // A particle the generator emitted as final state (status 1), the condition of
  // MtdTracksValidation::trkTPSelLV.
  [[nodiscard]] bool isGeneratorFinalState(truth::Graph const& graph, uint32_t particleId);

  // The cell of an MTD hit, row << 16 | col, and its SimHitCategory prodTypeMTD value.
  [[nodiscard]] inline uint32_t cellOf(uint32_t cellWord) { return cellWord & 0x00FFFFFFu; }
  [[nodiscard]] inline uint32_t categoryOf(uint32_t cellWord) { return cellWord >> 24; }

  // What one particle left on one sensor module in one category: the counterpart of a
  // MtdSimLayerCluster. Its time is the energy-weighted mean of the cell times, as
  // MtdSimLayerCluster::computeClusterTime defines it.
  struct Deposit {
    uint32_t module = 0;
    uint32_t category = 0;
    double energy = 0.;           // GeV
    double time = 0.;             // ns
    std::vector<uint32_t> cells;  // row << 16 | col, sorted

    [[nodiscard]] bool isBarrel() const;
    [[nodiscard]] int etlDisc() const;  // 1 or 2, 0 for BTL
    [[nodiscard]] bool sharesCell(uint32_t module, std::vector<uint32_t> const& sortedCells) const;
  };

  // The deposits of one particle, from LogicalGraphHitIndex::directHits and
  // directHitTimes of the MTD channel. Empty unless the channel is cell keyed and timed.
  [[nodiscard]] std::vector<Deposit> deposits(std::span<const truth::LogicalGraphHitIndex::Hit> hits,
                                              std::span<const float> times);

  // The rule of MtdRecoClusterToSimLayerClusterAssociatorByHits: a reco cluster matches
  // a deposit with a shared cell, a compatible time and, in BTL, a compatible energy.
  struct ClusterMatchCuts {
    double energyRatioMax = 5.;  // BTL, reco energy over sim energy
    double timePullMax = 10.;
  };
  [[nodiscard]] bool matches(Deposit const& deposit,
                             uint32_t module,
                             std::vector<uint32_t> const& sortedCells,
                             double recoEnergyGeV,
                             double recoTime,
                             double recoTimeError,
                             ClusterMatchCuts const& cuts);

}  // namespace tga::mtd

#endif
