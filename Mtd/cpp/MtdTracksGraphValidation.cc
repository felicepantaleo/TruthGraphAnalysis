// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// The track-to-MTD truth classification of MtdTracksValidation, with the truth read from
// the graph. The reco side is the same: the same tracks, the same MTD clusters on each
// track, the same time. Only the questions asked of the truth change source:
//
//   which particle made the track    TrackingParticle association -> graph track map
//   the particle selection           TrackingParticle fields      -> graph particle
//   what it left in the MTD          MtdSimLayerCluster to TP     -> MTD hit channel
//   which reco cluster is its own    reco to sim cluster map      -> shared cell, time, energy
//
// The monitor elements carry the names of their MtdTracksValidation counterparts, so the
// two folders compare name by name.

#include <algorithm>
#include <array>
#include <iterator>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/FTLRecHit/interface/FTLCluster.h"
#include "DataFormats/ForwardDetId/interface/ETLDetId.h"
#include "DataFormats/ForwardDetId/interface/MTDDetId.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackerRecHit2D/interface/MTDTrackingRecHit.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "SimDataFormats/Associations/interface/TICLAssociationMap.h"
#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/LogicalGraphHitIndex.h"
#include "TruthGraphAnalysis/Mtd/cpp/MtdTruth.h"

class MtdTracksGraphValidation : public DQMEDAnalyzer {
public:
  explicit MtdTracksGraphValidation(edm::ParameterSet const& pset);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  using TrackMap = ticl::TICLAssociationMap<ticl::mapWithSharedEnergyAndScore>;

  // One MTD cluster on a track, in the terms a Deposit is matched with.
  struct TrackCluster {
    uint32_t module = 0;
    std::vector<uint32_t> cells;  // row << 16 | col, sorted
    double energy = 0.;           // GeV
    double time = 0.;             // ns
    double timeError = 0.;        // ns
    int etlDisc = 0;              // 1 or 2, 0 for BTL
  };

  void bookHistograms(DQMStore::IBooker& ibook, edm::Run const&, edm::EventSetup const&) override;
  void analyze(edm::Event const& event, edm::EventSetup const&) override;

  // The best in-time particle of a track above the purity cut, as
  // MtdTracksValidation::getMatchedTP takes the first in-time TrackingParticle.
  [[nodiscard]] std::optional<uint32_t> matchedParticle(TrackMap const& map,
                                                        truth::Graph const& graph,
                                                        unsigned int track) const;
  [[nodiscard]] bool matchesAny(tga::mtd::Deposit const& deposit, std::vector<TrackCluster> const& clusters) const;

  const std::string folder_;
  const double trackMaxBtlEta_;
  const double recoEtaMax_;
  const double recoPtMin_;
  const float maxScore_;
  const tga::mtd::TruthSelection truthCuts_;
  const tga::mtd::ClusterMatchCuts clusterCuts_;

  const edm::EDGetTokenT<reco::TrackCollection> tracksToken_;
  const edm::EDGetTokenT<reco::TrackCollection> mtdTracksToken_;
  const edm::EDGetTokenT<edm::ValueMap<int>> trackAssocToken_;
  const edm::EDGetTokenT<edm::ValueMap<float>> t0Token_;
  const edm::EDGetTokenT<edm::ValueMap<float>> sigmat0Token_;
  const edm::EDGetTokenT<truth::Graph> graphToken_;
  const edm::EDGetTokenT<truth::LogicalGraphHitIndex> mtdHitIndexToken_;
  const edm::EDGetTokenT<TrackMap> trackMapToken_;

  MonitorElement* meBTLTot_ = nullptr;
  MonitorElement* meBTLDirect_ = nullptr;
  MonitorElement* meBTLOther_ = nullptr;
  MonitorElement* meBTLnomtd_ = nullptr;
  MonitorElement* meBTLDirectCorrect_ = nullptr;
  MonitorElement* meBTLDirectWrong_ = nullptr;
  std::array<MonitorElement*, 3> meBTLDirectWrongByOrigin_{};
  MonitorElement* meBTLDirectNoAssoc_ = nullptr;
  MonitorElement* meBTLOtherCorrect_ = nullptr;
  MonitorElement* meBTLOtherWrong_ = nullptr;
  MonitorElement* meBTLOtherNoAssoc_ = nullptr;
  MonitorElement* meBTLDirectCorrectTimeRes_ = nullptr;
  MonitorElement* meETLTot_ = nullptr;
  MonitorElement* meETLmtd_ = nullptr;
  MonitorElement* meETLnomtd_ = nullptr;
  MonitorElement* meETLCorrect_ = nullptr;
  MonitorElement* meETLWrong_ = nullptr;
  MonitorElement* meETLNoAssoc_ = nullptr;
  MonitorElement* meETLCorrectTimeRes_ = nullptr;
  bool reportedUnusableIndex_ = false;
};

MtdTracksGraphValidation::MtdTracksGraphValidation(edm::ParameterSet const& pset)
    : folder_(pset.getParameter<std::string>("folder")),
      trackMaxBtlEta_(pset.getParameter<double>("trackMaximumBtlEta")),
      recoEtaMax_(pset.getParameter<double>("trackMaximumEta")),
      recoPtMin_(pset.getParameter<double>("trackMinimumPt")),
      maxScore_(pset.getParameter<double>("maxScore")),
      truthCuts_{pset.getParameter<double>("truthMaximumEta"),
                 pset.getParameter<double>("truthMinimumPt"),
                 pset.getParameter<double>("truthMaximumProductionRho"),
                 pset.getParameter<double>("truthMaximumProductionZ")},
      clusterCuts_{pset.getParameter<double>("clusterEnergyCut"), pset.getParameter<double>("clusterTimeCut")},
      tracksToken_(consumes(pset.getParameter<edm::InputTag>("inputTagG"))),
      mtdTracksToken_(consumes(pset.getParameter<edm::InputTag>("inputTagT"))),
      trackAssocToken_(consumes(pset.getParameter<edm::InputTag>("trackAssocSrc"))),
      t0Token_(consumes(pset.getParameter<edm::InputTag>("t0SafePID"))),
      sigmat0Token_(consumes(pset.getParameter<edm::InputTag>("sigmat0SafePID"))),
      graphToken_(consumes(pset.getParameter<edm::InputTag>("graph"))),
      mtdHitIndexToken_(consumes(pset.getParameter<edm::InputTag>("mtdHitIndex"))),
      trackMapToken_(consumes(pset.getParameter<edm::InputTag>("trackToTruth"))) {}

void MtdTracksGraphValidation::bookHistograms(DQMStore::IBooker& ibook, edm::Run const&, edm::EventSetup const&) {
  ibook.setCurrentFolder(folder_);
  auto btlEta = [&ibook](const char* name, const char* title) { return ibook.book1D(name, title, 30, 0., 1.5); };
  auto etlEta = [&ibook](const char* name, const char* title) { return ibook.book1D(name, title, 30, 1.5, 3.0); };
  auto timeRes = [&ibook](const char* name, const char* title) { return ibook.book1D(name, title, 120, -0.15, 0.15); };

  meBTLTot_ = btlEta("BTLTrackMatchedTPEtaTot", "Tracks matched to a truth particle;#eta_{RECO}");
  meBTLDirect_ = btlEta("BTLTrackMatchedTPmtdDirectEta", "Particle with direct BTL hits;#eta_{RECO}");
  meBTLOther_ = btlEta("BTLTrackMatchedTPmtdOtherEta", "Particle with only other BTL hits;#eta_{RECO}");
  meBTLnomtd_ = btlEta("BTLTrackMatchedTPnomtdEta", "Particle without BTL hits;#eta_{RECO}");
  meBTLDirectCorrect_ =
      btlEta("BTLTrackMatchedTPmtdDirectCorrectAssocEta", "Track cluster is the first direct deposit;#eta_{RECO}");
  meBTLDirectWrong_ =
      btlEta("BTLTrackMatchedTPmtdDirectWrongAssocEta", "Track cluster is not the first direct deposit;#eta_{RECO}");
  meBTLDirectWrongByOrigin_ = {
      btlEta("BTLTrackMatchedTPmtdDirectWrongAssocEta1", "Wrong cluster: a later direct deposit;#eta_{RECO}"),
      btlEta("BTLTrackMatchedTPmtdDirectWrongAssocEta2", "Wrong cluster: an other deposit;#eta_{RECO}"),
      btlEta("BTLTrackMatchedTPmtdDirectWrongAssocEta3", "Wrong cluster: not this particle;#eta_{RECO}")};
  meBTLDirectNoAssoc_ = btlEta("BTLTrackMatchedTPmtdDirectNoAssocEta", "Track with no BTL cluster;#eta_{RECO}");
  meBTLOtherCorrect_ =
      btlEta("BTLTrackMatchedTPmtdOtherCorrectAssocEta", "Track cluster is an other deposit;#eta_{RECO}");
  meBTLOtherWrong_ = btlEta("BTLTrackMatchedTPmtdOtherWrongAssocEta", "Track cluster not made by it;#eta_{RECO}");
  meBTLOtherNoAssoc_ = btlEta("BTLTrackMatchedTPmtdOtherNoAssocEta", "Track with no BTL cluster;#eta_{RECO}");
  meBTLDirectCorrectTimeRes_ =
      timeRes("BTLTrackMatchedTPmtdDirectCorrectAssocTimeRes", "Correct track-MTD association;t_{rec} - t_{sim} [ns]");

  meETLTot_ = etlEta("ETLTrackMatchedTPEtaTot", "Tracks matched to a truth particle;#eta_{RECO}");
  meETLmtd_ = etlEta("ETLTrackMatchedTPmtd1Eta", "Particle with ETL hits;#eta_{RECO}");
  meETLnomtd_ = etlEta("ETLTrackMatchedTPnomtdEta", "Particle without ETL hits;#eta_{RECO}");
  meETLCorrect_ = etlEta("ETLTrackMatchedTPmtd1CorrectAssocEta", "Every disc cluster made by it;#eta_{RECO}");
  meETLWrong_ = etlEta("ETLTrackMatchedTPmtd1WrongAssocEta", "A disc cluster not made by it;#eta_{RECO}");
  meETLNoAssoc_ = etlEta("ETLTrackMatchedTPmtd1NoAssocEta", "Track with no ETL cluster;#eta_{RECO}");
  meETLCorrectTimeRes_ =
      timeRes("ETLTrackMatchedTPmtd1CorrectAssocTimeRes", "Correct track-MTD association;t_{rec} - t_{sim} [ns]");
}

std::optional<uint32_t> MtdTracksGraphValidation::matchedParticle(TrackMap const& map,
                                                                  truth::Graph const& graph,
                                                                  unsigned int track) const {
  if (track >= map.size())
    return std::nullopt;
  // A row is sorted by score, and the score is 1 - purity.
  for (auto const& element : map[track]) {
    if (element.score() > maxScore_)
      break;
    if (graph.particles()[element.index()].bunchCrossing() == 0)
      return element.index();
  }
  return std::nullopt;
}

bool MtdTracksGraphValidation::matchesAny(tga::mtd::Deposit const& deposit,
                                          std::vector<TrackCluster> const& clusters) const {
  return std::any_of(clusters.begin(), clusters.end(), [&](TrackCluster const& c) {
    return tga::mtd::matches(deposit, c.module, c.cells, c.energy, c.time, c.timeError, clusterCuts_);
  });
}

void MtdTracksGraphValidation::analyze(edm::Event const& event, edm::EventSetup const&) {
  const auto tracksHandle = event.getHandle(tracksToken_);
  auto const& mtdTracks = event.get(mtdTracksToken_);
  auto const& trackAssoc = event.get(trackAssocToken_);
  auto const& t0 = event.get(t0Token_);
  auto const& sigmat0 = event.get(sigmat0Token_);
  auto const& graph = event.get(graphToken_);
  auto const& mtdHits = event.get(mtdHitIndexToken_);
  auto const& trackMap = event.get(trackMapToken_);

  for (unsigned int index = 0; index < tracksHandle->size(); ++index) {
    const reco::TrackRef trackRef(tracksHandle, index);
    auto const& trackGen = *trackRef;
    if (trackAssoc[trackRef] == -1)
      continue;
    if (std::abs(trackGen.eta()) > recoEtaMax_ || trackGen.pt() <= recoPtMin_)
      continue;

    const auto particle = matchedParticle(trackMap, graph, index);
    if (!particle || !tga::mtd::passesSelection(graph, *particle, truthCuts_))
      continue;
    const auto hits = mtdHits.directHits(truth::HitChannel::MTD, *particle);
    const auto times = mtdHits.directHitTimes(truth::HitChannel::MTD, *particle);
    // Without cells and times every track would read as "no MTD hits", so say so.
    if (!hits.empty() && (times.size() != hits.size() || !mtdHits.isCellKeyed(truth::HitChannel::MTD)) &&
        !reportedUnusableIndex_) {
      edm::LogError("MtdTracksGraphValidation")
          << "the MTD channel of the hit index carries no cells or no times, so no deposit can be read. "
             "Build it with TruthLogicalGraphHitIndexProducer and subdetectors = [\"MTD\"].";
      reportedUnusableIndex_ = true;
    }
    const auto deposits = tga::mtd::deposits(hits, times);

    // The MTD clusters on the extended track.
    bool btlHit = false, etlHit = false;
    std::vector<TrackCluster> btlClusters, etlClusters;
    for (auto const* hit : mtdTracks[trackAssoc[trackRef]].recHits()) {
      if (!hit->isValid())
        continue;
      const MTDDetId id(hit->geographicalId());
      if (id.det() != DetId::Forward || id.subdetId() != MTDDetId::FastTime)
        continue;
      const bool barrel = id.mtdSubDetector() == MTDDetId::BTL;
      (barrel ? btlHit : etlHit) = true;
      auto const& cluster = static_cast<MTDTrackingRecHit const*>(hit)->mtdCluster();
      if (cluster.size() == 0)
        continue;
      TrackCluster out;
      out.module = cluster.id();
      for (int i = 0; i < cluster.size(); ++i) {
        const uint32_t row = cluster.minHitRow() + cluster.hitOffset()[i * 2];
        const uint32_t col = cluster.minHitCol() + cluster.hitOffset()[i * 2 + 1];
        out.cells.push_back(row << 16 | col);
      }
      std::sort(out.cells.begin(), out.cells.end());
      out.energy = cluster.energy() * 0.001;  // MeV to GeV, the unit of the sim energy
      out.time = cluster.time();
      out.timeError = cluster.timeError();
      out.etlDisc = barrel ? 0 : ETLDetId(id).nDisc();
      (barrel ? btlClusters : etlClusters).push_back(std::move(out));
    }

    const float eta = std::abs(trackGen.eta());
    const bool hasTime = sigmat0[trackRef] != -1.f;
    const auto point = tga::mtd::productionPoint(graph, *particle);
    const double dT = hasTime && point ? t0[trackRef] - point->T() : 0.;

    if (eta < trackMaxBtlEta_) {
      meBTLTot_->Fill(eta);
      // The first direct BTL deposit in time is the one the legacy module calls correct.
      // Any other deposit, in BTL or ETL, makes the particle an "other" one, as there.
      tga::mtd::Deposit const* firstDirect = nullptr;
      bool hasOther = false;
      for (auto const& deposit : deposits) {
        if (deposit.category != 0)
          hasOther = true;
        else if (deposit.isBarrel() && (firstDirect == nullptr || deposit.time < firstDirect->time))
          firstDirect = &deposit;
      }
      bool laterDirect = false, other = false;
      for (auto const& deposit : deposits) {
        if (!deposit.isBarrel() || &deposit == firstDirect || !matchesAny(deposit, btlClusters))
          continue;
        (deposit.category == 0 ? laterDirect : other) = true;
      }
      if (firstDirect != nullptr) {
        meBTLDirect_->Fill(eta);
        if (!btlHit) {
          meBTLDirectNoAssoc_->Fill(eta);
        } else if (matchesAny(*firstDirect, btlClusters)) {
          meBTLDirectCorrect_->Fill(eta);
          if (hasTime)
            meBTLDirectCorrectTimeRes_->Fill(dT);
        } else {
          meBTLDirectWrong_->Fill(eta);
          meBTLDirectWrongByOrigin_[laterDirect ? 0 : (other ? 1 : 2)]->Fill(eta);
        }
      } else if (hasOther) {
        meBTLOther_->Fill(eta);
        if (!btlHit)
          meBTLOtherNoAssoc_->Fill(eta);
        else if (other)
          meBTLOtherCorrect_->Fill(eta);
        else
          meBTLOtherWrong_->Fill(eta);
      } else {
        meBTLnomtd_->Fill(eta);
      }
      continue;
    }

    meETLTot_->Fill(eta);
    bool d1 = false, d2 = false, ownDisc1 = false, ownDisc2 = false;
    for (auto const& deposit : deposits) {
      const int disc = deposit.etlDisc();
      if (disc == 0)
        continue;
      (disc == 1 ? d1 : d2) = true;
      std::vector<TrackCluster> sameDisc;
      std::copy_if(etlClusters.begin(), etlClusters.end(), std::back_inserter(sameDisc), [disc](TrackCluster const& c) {
        return c.etlDisc == disc;
      });
      if (matchesAny(deposit, sameDisc))
        (disc == 1 ? ownDisc1 : ownDisc2) = true;
    }
    if (!d1 && !d2) {
      meETLnomtd_->Fill(eta);
      continue;
    }
    meETLmtd_->Fill(eta);
    if (!etlHit) {
      meETLNoAssoc_->Fill(eta);
      continue;
    }
    const bool etlDisc1 =
        std::any_of(etlClusters.begin(), etlClusters.end(), [](auto const& c) { return c.etlDisc == 1; });
    const bool etlDisc2 =
        std::any_of(etlClusters.begin(), etlClusters.end(), [](auto const& c) { return c.etlDisc == 2; });
    // Every disc the particle crossed has a cluster on the track, and that cluster is its
    // own: the rule of MtdTracksValidation for the ETL.
    const bool correct = (d1 && !d2 && etlDisc1 && ownDisc1) || (d2 && !d1 && etlDisc2 && ownDisc2) ||
                         (d1 && d2 && etlDisc1 && etlDisc2 && ownDisc1 && ownDisc2);
    if (correct) {
      meETLCorrect_->Fill(eta);
      if (hasTime)
        meETLCorrectTimeRes_->Fill(dT);
    } else if ((d1 && !ownDisc1) || (d2 && !ownDisc2)) {
      meETLWrong_->Fill(eta);
    }
  }
}

void MtdTracksGraphValidation::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("folder", "MTD/TracksGraph");
  desc.add<edm::InputTag>("inputTagG", edm::InputTag("generalTracks"));
  desc.add<edm::InputTag>("inputTagT", edm::InputTag("trackExtenderWithMTD"));
  desc.add<edm::InputTag>("trackAssocSrc", edm::InputTag("trackExtenderWithMTD:generalTrackassoc"));
  desc.add<edm::InputTag>("t0SafePID", edm::InputTag("tofPID:t0safe"));
  desc.add<edm::InputTag>("sigmat0SafePID", edm::InputTag("tofPID:sigmat0safe"));
  desc.add<edm::InputTag>("graph", edm::InputTag("truthLogicalGraphProducer"));
  desc.add<edm::InputTag>("mtdHitIndex", edm::InputTag("truthMtdHitIndex"));
  desc.add<edm::InputTag>("trackToTruth", edm::InputTag("mtdTrackToTruth", "generalTracksRecoToTruthFixed"));
  desc.add<double>("trackMaximumBtlEta", 1.5);
  desc.add<double>("trackMaximumEta", 3.);
  desc.add<double>("trackMinimumPt", 0.7);
  desc.add<double>("maxScore", 0.25)
      ->setComment(
          "Highest score, 1 - purity, of a track-to-particle match. 0.25 is the 75% hit purity "
          "QuickTrackAssociatorByHits asks for.");
  desc.add<double>("truthMaximumEta", 4.);
  desc.add<double>("truthMinimumPt", 0.7);
  desc.add<double>("truthMaximumProductionRho", 110.);
  desc.add<double>("truthMaximumProductionZ", 290.);
  desc.add<double>("clusterEnergyCut", 5.)
      ->setComment("BTL: highest reco over sim energy of a matched cluster, as the MTD reco-to-sim association.");
  desc.add<double>("clusterTimeCut", 10.)
      ->setComment("Highest time pull of a matched cluster, as the MTD reco-to-sim association.");
  descriptions.add("mtdTracksGraphValid", desc);
}

DEFINE_FWK_MODULE(MtdTracksGraphValidation);
