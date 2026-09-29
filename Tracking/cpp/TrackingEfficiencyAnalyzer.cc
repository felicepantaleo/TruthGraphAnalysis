// Track efficiency, fake rate and duplicate rate from the truth graph, with the
// TrackingParticle selection and the 75% hit purity of MultiTrackValidator. Reads the
// association maps that customiseTruthGraphAssociators writes into step3.root.
#include <cmath>
#include <cstdint>
#include <string>

#include "TH1D.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
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
#include "SimDataFormats/TruthInfo/interface/Vertex.h"

namespace {
  using BranchMap = ticl::TICLAssociationMap<ticl::mapWithSharedEnergyAndScore>;

  // Fraction of the track's valid hits that the particle owns.
  float purity(float sharedHits, reco::Track const& track) {
    return track.numberOfValidHits() > 0 ? sharedHits / track.numberOfValidHits() : 0.f;
  }
}  // namespace

class TrackingEfficiencyAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit TrackingEfficiencyAnalyzer(edm::ParameterSet const& cfg);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(edm::Event const& event, edm::EventSetup const&) override;
  void endJob() override;
  bool selected(truth::Particle const& particle) const;
  bool fromParticle(float sharedHits, reco::Track const& track) const;

  const edm::EDGetTokenT<truth::Graph> graphToken_;
  const edm::EDGetTokenT<reco::TrackCollection> tracksToken_;
  const edm::EDGetTokenT<BranchMap> truthToRecoToken_;
  const edm::EDGetTokenT<BranchMap> recoToTruthToken_;
  const double ptMin_, maxAbsEta_, maxVertexR_, maxVertexZ_, minPurity_;
  const bool threeHitTracksNeedAllHits_;

  TH1D* simulated_;
  TH1D* found_;
  TH1D* duplicated_;
  TH1D* reconstructed_;
  TH1D* matched_;
  TH1D* simulatedPt_;
  TH1D* foundPt_;
  TH1D* reconstructedPt_;
  TH1D* matchedPt_;
};

TrackingEfficiencyAnalyzer::TrackingEfficiencyAnalyzer(edm::ParameterSet const& cfg)
    : graphToken_(consumes(cfg.getParameter<edm::InputTag>("graph"))),
      tracksToken_(consumes(cfg.getParameter<edm::InputTag>("tracks"))),
      truthToRecoToken_(consumes(cfg.getParameter<edm::InputTag>("truthToReco"))),
      recoToTruthToken_(consumes(cfg.getParameter<edm::InputTag>("recoToTruth"))),
      ptMin_(cfg.getParameter<double>("ptMin")),
      maxAbsEta_(cfg.getParameter<double>("maxAbsEta")),
      maxVertexR_(cfg.getParameter<double>("maxVertexR")),
      maxVertexZ_(cfg.getParameter<double>("maxVertexZ")),
      minPurity_(cfg.getParameter<double>("minPurity")),
      threeHitTracksNeedAllHits_(cfg.getParameter<bool>("threeHitTracksNeedAllHits")) {
  usesResource(TFileService::kSharedResource);
  edm::Service<TFileService> fs;
  simulated_ = fs->make<TH1D>("simulated_eta", ";truth #eta;selected particles", 90, -4.5, 4.5);
  found_ = fs->make<TH1D>("found_eta", ";truth #eta;particles with a track", 90, -4.5, 4.5);
  duplicated_ = fs->make<TH1D>("duplicated_eta", ";truth #eta;particles with two or more tracks", 90, -4.5, 4.5);
  reconstructed_ = fs->make<TH1D>("reconstructed_eta", ";track #eta;tracks", 90, -4.5, 4.5);
  matched_ = fs->make<TH1D>("matched_eta", ";track #eta;tracks from one particle", 90, -4.5, 4.5);
  simulatedPt_ = fs->make<TH1D>("simulated_pt", ";truth p_{T} [GeV];selected particles", 100, 0., 10.);
  foundPt_ = fs->make<TH1D>("found_pt", ";truth p_{T} [GeV];particles with a track", 100, 0., 10.);
  reconstructedPt_ = fs->make<TH1D>("reconstructed_pt", ";track p_{T} [GeV];tracks", 100, 0., 10.);
  matchedPt_ = fs->make<TH1D>("matched_pt", ";track p_{T} [GeV];tracks from one particle", 100, 0., 10.);
}

void TrackingEfficiencyAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setComment(
      "Track efficiency, fake and duplicate rate from the truth graph, with the MultiTrackValidator selection");
  desc.add<edm::InputTag>("graph", edm::InputTag("truthLogicalGraphProducer"));
  desc.add<edm::InputTag>("tracks", edm::InputTag("generalTracks"));
  desc.add<edm::InputTag>("truthToReco", edm::InputTag("allTrackToTruthBranchAssociators", "generalTracksTruthToReco"))
      ->setComment("one row per particle: (track, shared hits, score)");
  desc.add<edm::InputTag>("recoToTruth",
                          edm::InputTag("allTrackToTruthBranchAssociators", "generalTracksRecoToTruthFixed"))
      ->setComment("one row per track: (particle, shared hits, score), detector particles first");
  desc.add<double>("ptMin", 0.9);
  desc.add<double>("maxAbsEta", 4.5);
  desc.add<double>("maxVertexR", 2.5)->setComment("production vertex radius, cm");
  desc.add<double>("maxVertexZ", 30.)->setComment("production vertex |z|, cm");
  desc.add<double>("minPurity", 0.75)->setComment("minimum fraction of the track hits that the particle owns");
  desc.add<bool>("threeHitTracksNeedAllHits", true)
      ->setComment("a track with three hits comes from a particle only when the particle owns all three");
  descriptions.add("trackingEfficiencyAnalyzer", desc);
}

// The TrackingParticle selection of the MultiTrackValidator efficiency: a simulated,
// charged, in-time signal particle, produced near the beam line.
bool TrackingEfficiencyAnalyzer::selected(truth::Particle const& particle) const {
  if (!particle.hasSim() || !particle.isSignal() || particle.charge() == 0.)
    return false;
  auto const& p4 = particle.momentum();
  if (p4.pt() < ptMin_ || std::abs(p4.eta()) > maxAbsEta_)
    return false;
  auto const production = particle.productionVertices();
  if (production.empty())
    return false;
  auto const& x = production.front().position();
  return std::hypot(x.x(), x.y()) < maxVertexR_ && std::abs(x.z()) < maxVertexZ_;
}

// A track comes from a particle when the particle owns enough of its hits.
bool TrackingEfficiencyAnalyzer::fromParticle(float sharedHits, reco::Track const& track) const {
  if (threeHitTracksNeedAllHits_ && track.numberOfValidHits() == 3)
    return sharedHits >= 3.f;
  return purity(sharedHits, track) >= minPurity_;
}

void TrackingEfficiencyAnalyzer::analyze(edm::Event const& event, edm::EventSetup const&) {
  auto const& graph = event.get(graphToken_);
  auto const& tracks = event.get(tracksToken_);
  auto const& truthToReco = event.get(truthToRecoToken_).getMap();
  auto const& recoToTruth = event.get(recoToTruthToken_).getMap();

  // Efficiency and duplicates: count the pure tracks of each selected particle.
  for (uint32_t id = 0; id < graph.nParticles(); ++id) {
    const truth::Particle particle(&graph, id);
    if (!selected(particle))
      continue;
    const double eta = particle.momentum().eta(), pt = particle.momentum().pt();
    simulated_->Fill(eta);
    simulatedPt_->Fill(pt);
    unsigned int pureTracks = 0;
    if (id < truthToReco.size()) {
      for (auto const& match : truthToReco[id]) {
        if (fromParticle(match.sharedEnergy(), tracks[match.index()]))
          ++pureTracks;
      }
    }
    if (pureTracks > 0) {
      found_->Fill(eta);
      foundPt_->Fill(pt);
    }
    if (pureTracks > 1)
      duplicated_->Fill(eta);
  }

  // Fake rate: a track is fake when no particle owns 75% of its hits. Row [0] of the
  // Fixed map is the best detector particle.
  for (uint32_t t = 0; t < tracks.size(); ++t) {
    reconstructed_->Fill(tracks[t].eta());
    reconstructedPt_->Fill(tracks[t].pt());
    if (t < recoToTruth.size() && !recoToTruth[t].empty() &&
        fromParticle(recoToTruth[t].front().sharedEnergy(), tracks[t])) {
      matched_->Fill(tracks[t].eta());
      matchedPt_->Fill(tracks[t].pt());
    }
  }
}

void TrackingEfficiencyAnalyzer::endJob() {
  const double nSim = simulated_->GetEntries(), nReco = reconstructed_->GetEntries();
  edm::LogPrint("TrackingEfficiencyAnalyzer")
      << "selected particles " << nSim << ", efficiency " << found_->GetEntries() / nSim << ", duplicate rate "
      << duplicated_->GetEntries() / nSim << "; tracks " << nReco << ", fake rate "
      << 1. - matched_->GetEntries() / nReco;
}

DEFINE_FWK_MODULE(TrackingEfficiencyAnalyzer);
