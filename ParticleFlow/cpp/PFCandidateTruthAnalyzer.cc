// Checks the identity and the energy of PF candidates against the truth graph. A charged
// candidate takes the particle of its track, from the track map. A neutral candidate
// takes the particle of its most energetic ECAL or HCAL cluster, from the PF cluster maps.
#include <array>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <string>

#include "TH1D.h"
#include "TH2D.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "SimDataFormats/Associations/interface/TICLAssociationMap.h"
#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/Particle.h"

namespace {
  using BranchMap = ticl::TICLAssociationMap<ticl::mapWithSharedEnergyAndScore>;

  // Truth classes, the same for charged and neutral candidates.
  enum TruthClass { kElectron, kMuon, kPhoton, kChargedHadron, kNeutralHadron, kMerged, kNoMatch, kNTruth };
  const std::array<std::string, kNTruth> kTruthNames = {"e", "mu", "gamma", "h+-", "h0", "merged", "no match"};

  TruthClass truthClass(truth::Particle const& particle) {
    // A particle that the generator decayed, a pi0 or a B, is the best match when the
    // object merges several of its decay products.
    if (!particle.hasSim())
      return kMerged;
    const int pdg = std::abs(particle.pdgId());
    if (pdg == 11)
      return kElectron;
    if (pdg == 13)
      return kMuon;
    if (pdg == 22)
      return kPhoton;
    return particle.charge() != 0. ? kChargedHadron : kNeutralHadron;
  }
}  // namespace

class PFCandidateTruthAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit PFCandidateTruthAnalyzer(edm::ParameterSet const& cfg);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(edm::Event const& event, edm::EventSetup const&) override;
  void endJob() override;
  void fill(reco::PFCandidate const& candidate,
            ticl::mapWithSharedEnergyAndScore const& map,
            uint32_t row,
            truth::Graph const& graph);

  const edm::EDGetTokenT<truth::Graph> graphToken_;
  const edm::EDGetTokenT<reco::PFCandidateCollection> candidatesToken_;
  const edm::EDGetTokenT<reco::TrackCollection> tracksToken_;
  const edm::EDGetTokenT<BranchMap> trackMapToken_;
  const edm::EDGetTokenT<reco::PFClusterCollection> ecalClustersToken_;
  const edm::EDGetTokenT<BranchMap> ecalMapToken_;
  const edm::EDGetTokenT<reco::PFClusterCollection> hcalClustersToken_;
  const edm::EDGetTokenT<BranchMap> hcalMapToken_;
  const double maxScore_, minEnergy_;

  TH2D* identity_;
  TH1D* chargedResponse_;
  TH1D* neutralResponse_;
  unsigned long nNoCluster_ = 0;
};

PFCandidateTruthAnalyzer::PFCandidateTruthAnalyzer(edm::ParameterSet const& cfg)
    : graphToken_(consumes(cfg.getParameter<edm::InputTag>("graph"))),
      candidatesToken_(consumes(cfg.getParameter<edm::InputTag>("candidates"))),
      tracksToken_(consumes(cfg.getParameter<edm::InputTag>("tracks"))),
      trackMapToken_(consumes(cfg.getParameter<edm::InputTag>("trackMap"))),
      ecalClustersToken_(consumes(cfg.getParameter<edm::InputTag>("ecalClusters"))),
      ecalMapToken_(consumes(cfg.getParameter<edm::InputTag>("ecalMap"))),
      hcalClustersToken_(consumes(cfg.getParameter<edm::InputTag>("hcalClusters"))),
      hcalMapToken_(consumes(cfg.getParameter<edm::InputTag>("hcalMap"))),
      maxScore_(cfg.getParameter<double>("maxScore")),
      minEnergy_(cfg.getParameter<double>("minEnergy")) {
  usesResource(TFileService::kSharedResource);
  edm::Service<TFileService> fs;
  identity_ = fs->make<TH2D>("identity", ";PF candidate type;truth class", 7, 0., 7., kNTruth, 0., kNTruth);
  for (int t = 0; t < kNTruth; ++t)
    identity_->GetYaxis()->SetBinLabel(t + 1, kTruthNames[t].c_str());
  const std::array<std::string, 7> pfNames = {"X", "h", "e", "mu", "gamma", "h0", "h_HF"};
  for (int p = 0; p < 7; ++p)
    identity_->GetXaxis()->SetBinLabel(p + 1, pfNames[p].c_str());
  chargedResponse_ = fs->make<TH1D>("charged_response", ";E^{PF} / E^{truth};charged candidates", 60, 0., 2.);
  neutralResponse_ = fs->make<TH1D>("neutral_response", ";E^{PF} / E^{truth};neutral candidates", 60, 0., 2.);
}

void PFCandidateTruthAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setComment("PF candidate identity and energy against the truth graph");
  desc.add<edm::InputTag>("graph", edm::InputTag("truthLogicalGraphProducer"));
  desc.add<edm::InputTag>("candidates", edm::InputTag("particleFlow"));
  desc.add<edm::InputTag>("tracks", edm::InputTag("generalTracks"));
  desc.add<edm::InputTag>("trackMap",
                          edm::InputTag("allTrackToTruthBranchAssociators", "generalTracksRecoToTruthFixed"));
  desc.add<edm::InputTag>("ecalClusters", edm::InputTag("particleFlowClusterECAL"));
  desc.add<edm::InputTag>(
      "ecalMap", edm::InputTag("truthBranchPFClusterEcalAssociators", "particleFlowClusterECALRecoToTruthFixed"));
  desc.add<edm::InputTag>("hcalClusters", edm::InputTag("particleFlowClusterHCAL"));
  desc.add<edm::InputTag>(
      "hcalMap", edm::InputTag("truthBranchPFClusterHcalAssociators", "particleFlowClusterHCALRecoToTruthFixed"));
  desc.add<double>("maxScore", 0.25)->setComment("a match counts when its score, 1 - purity, is at most this");
  desc.add<double>("minEnergy", 0.)->setComment("candidates below this energy are skipped, GeV");
  descriptions.add("pfCandidateTruthAnalyzer", desc);
}

// Row [0] of a Fixed map is the best detector particle.
void PFCandidateTruthAnalyzer::fill(reco::PFCandidate const& candidate,
                                    ticl::mapWithSharedEnergyAndScore const& map,
                                    uint32_t row,
                                    truth::Graph const& graph) {
  const bool charged = candidate.charge() != 0;
  if (row >= map.size() || map[row].empty() || map[row].front().score() > maxScore_) {
    identity_->Fill(candidate.particleId(), kNoMatch);
    return;
  }
  const truth::Particle particle(&graph, map[row].front().index());
  identity_->Fill(candidate.particleId(), truthClass(particle));
  (charged ? chargedResponse_ : neutralResponse_)->Fill(candidate.energy() / particle.momentum().energy());
}

void PFCandidateTruthAnalyzer::analyze(edm::Event const& event, edm::EventSetup const&) {
  auto const& graph = event.get(graphToken_);
  auto const tracks = event.getHandle(tracksToken_);
  auto const& trackMap = event.get(trackMapToken_).getMap();
  auto const ecalClusters = event.getHandle(ecalClustersToken_);
  auto const& ecalMap = event.get(ecalMapToken_).getMap();
  auto const hcalClusters = event.getHandle(hcalClustersToken_);
  auto const& hcalMap = event.get(hcalMapToken_).getMap();

  for (auto const& candidate : event.get(candidatesToken_)) {
    if (candidate.energy() < minEnergy_)
      continue;
    // Charged: the track of the candidate, when it is a generalTracks track.
    if (candidate.charge() != 0) {
      auto const track = candidate.trackRef();
      if (track.isNonnull() && track.id() == tracks.id())
        fill(candidate, trackMap, track.key(), graph);
      else
        identity_->Fill(candidate.particleId(), kNoMatch);
      continue;
    }
    // Neutral: the most energetic ECAL or HCAL cluster in the candidate blocks.
    reco::PFClusterRef best;
    for (auto const& [block, index] : candidate.elementsInBlocks()) {
      auto const& element = block->elements()[index];
      auto const cluster = element.clusterRef();
      if (cluster.isNull() || (cluster.id() != ecalClusters.id() && cluster.id() != hcalClusters.id()))
        continue;
      if (best.isNull() || cluster->energy() > best->energy())
        best = cluster;
    }
    if (best.isNull()) {
      ++nNoCluster_;
      continue;
    }
    fill(candidate, best.id() == ecalClusters.id() ? ecalMap : hcalMap, best.key(), graph);
  }
}

void PFCandidateTruthAnalyzer::endJob() {
  std::ostringstream table;
  table << "PF type vs truth class (rows: PF type)\n" << "      ";
  for (auto const& name : kTruthNames)
    table << std::setw(10) << name;
  for (int p = 1; p <= 5; ++p) {
    table << "\n" << std::setw(6) << identity_->GetXaxis()->GetBinLabel(p + 1);
    for (int t = 1; t <= kNTruth; ++t)
      table << std::setw(10) << identity_->GetBinContent(p + 1, t);
  }
  table << "\nneutral candidates without an ECAL or HCAL cluster (HGCAL or HF): " << nNoCluster_
        << "\nmean E response: charged " << chargedResponse_->GetMean() << ", neutral " << neutralResponse_->GetMean();
  edm::LogPrint("PFCandidateTruthAnalyzer") << table.str();
}

DEFINE_FWK_MODULE(PFCandidateTruthAnalyzer);
