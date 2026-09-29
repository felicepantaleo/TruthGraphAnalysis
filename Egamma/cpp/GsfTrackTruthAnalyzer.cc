// Associates GSF tracks to the truth graph with the hit associator, run in the job, and
// compares the hit-based match of the Z electrons with a delta R match.
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

#include "TH1D.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrackFwd.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "PhysicsTools/TruthInfo/interface/BranchHitAssociator.h"
#include "PhysicsTools/TruthInfo/interface/RecoHitAdapters.h"
#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/LogicalGraphHitIndex.h"
#include "SimDataFormats/TruthInfo/interface/Particle.h"

class GsfTrackTruthAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit GsfTrackTruthAnalyzer(edm::ParameterSet const& cfg);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(edm::Event const& event, edm::EventSetup const&) override;
  void endJob() override;

  const edm::EDGetTokenT<truth::Graph> graphToken_;
  const edm::EDGetTokenT<truth::LogicalGraphHitIndex> hitIndexToken_;
  const edm::EDGetTokenT<reco::GsfTrackCollection> gsfTracksToken_;
  const edm::EDGetTokenT<std::vector<unsigned int>> selectedRootsToken_;
  const edm::EDGetTokenT<std::vector<unsigned int>> assignableRootsToken_;
  const double minPurity_, electronPtMin_, electronMaxAbsEta_, maxDeltaR_;

  TH1D* purity_;
  TH1D* ptResponse_;
  unsigned long nTracks_ = 0, nPure_ = 0, nElectron_ = 0, nFromZ_ = 0;
  unsigned long nZElectrons_ = 0, nByHits_ = 0, nByDeltaR_ = 0, nByBoth_ = 0;
};

GsfTrackTruthAnalyzer::GsfTrackTruthAnalyzer(edm::ParameterSet const& cfg)
    : graphToken_(consumes(cfg.getParameter<edm::InputTag>("graph"))),
      hitIndexToken_(consumes(cfg.getParameter<edm::InputTag>("hitIndex"))),
      gsfTracksToken_(consumes(cfg.getParameter<edm::InputTag>("gsfTracks"))),
      selectedRootsToken_(consumes(cfg.getParameter<edm::InputTag>("selectedRoots"))),
      assignableRootsToken_(consumes(cfg.getParameter<edm::InputTag>("assignableRoots"))),
      minPurity_(cfg.getParameter<double>("minPurity")),
      electronPtMin_(cfg.getParameter<double>("electronPtMin")),
      electronMaxAbsEta_(cfg.getParameter<double>("electronMaxAbsEta")),
      maxDeltaR_(cfg.getParameter<double>("maxDeltaR")) {
  usesResource(TFileService::kSharedResource);
  edm::Service<TFileService> fs;
  purity_ = fs->make<TH1D>("purity", ";hit purity of the best particle;GSF tracks", 50, 0., 1.0001);
  ptResponse_ = fs->make<TH1D>("pt_response", ";p_{T}^{GSF} / p_{T}^{truth};Z electrons", 60, 0., 1.5);
}

void GsfTrackTruthAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setComment("GSF tracks against the truth graph: hit-based association and a delta R comparison");
  desc.add<edm::InputTag>("graph", edm::InputTag("truthLogicalGraphProducer"));
  desc.add<edm::InputTag>("hitIndex", edm::InputTag("truthLogicalGraphHitIndexProducer"));
  desc.add<edm::InputTag>("gsfTracks", edm::InputTag("electronGsfTracks"));
  desc.add<edm::InputTag>("selectedRoots", edm::InputTag("truthBranchTargets", "selectedRoots"))
      ->setComment("the candidate particles");
  desc.add<edm::InputTag>("assignableRoots", edm::InputTag("truthBranchTargets", "assignableRoots"))
      ->setComment("the candidates a reco object may be assigned to: no parton, boson or beam particle");
  desc.add<double>("minPurity", 0.75);
  desc.add<double>("electronPtMin", 5.);
  desc.add<double>("electronMaxAbsEta", 3.);
  desc.add<double>("maxDeltaR", 0.05);
  descriptions.add("gsfTrackTruthAnalyzer", desc);
}

void GsfTrackTruthAnalyzer::analyze(edm::Event const& event, edm::EventSetup const&) {
  auto const& graph = event.get(graphToken_);
  auto const& tracks = event.get(gsfTracksToken_);
  auto const& selected = event.get(selectedRootsToken_);
  std::vector<bool> assignable(graph.nParticles(), false);
  for (auto id : event.get(assignableRootsToken_))
    if (id < assignable.size())
      assignable[id] = true;

  // One associator per event, on the tracker channel, counting shared hits.
  const truth::BranchHitAssociator associator(event.get(hitIndexToken_),
                                              std::vector<uint32_t>(selected.begin(), selected.end()),
                                              truth::BranchHitAssociator::Metric::SharedHits,
                                              truth::HitChannel::Tracker,
                                              /*emptyRootsMeansAll=*/false);

  // The particle of each GSF track: the best assignable candidate, if it owns enough hits.
  std::vector<uint32_t> particleOfTrack(tracks.size(), truth::BranchMatch::kInvalidRoot);
  for (std::size_t t = 0; t < tracks.size(); ++t) {
    ++nTracks_;
    const auto hits = truth::recoHits(tracks[t]);
    for (auto const& match : associator.bestBranches(std::span<const truth::RecoHit>(hits))) {
      if (match.rootParticleId >= assignable.size() || !assignable[match.rootParticleId])
        continue;
      const double purity = 1. - match.score;
      purity_->Fill(purity);
      if (purity >= minPurity_) {
        // Two generator copies of one electron own the same hits and tie. The associator
        // then puts the earlier copy first, so take its last copy.
        const truth::Particle particle = truth::Particle(&graph, match.rootParticleId).lastCopy();
        particleOfTrack[t] = particle.id();
        ++nPure_;
        if (std::abs(particle.pdgId()) == 11) {
          ++nElectron_;
          if (particle.hasAncestorPdgId(23))
            ++nFromZ_;
        }
      }
      break;
    }
  }

  // The Z electrons: found by hits (a GSF track belongs to the electron) or by delta R.
  for (uint32_t id = 0; id < graph.nParticles(); ++id) {
    const truth::Particle electron(&graph, id);
    if (std::abs(electron.pdgId()) != 11 || !electron.hasGen() || electron.status() != 1 ||
        !electron.hasAncestorPdgId(23))
      continue;
    auto const& p4 = electron.momentum();
    if (p4.pt() < electronPtMin_ || std::abs(p4.eta()) > electronMaxAbsEta_)
      continue;
    ++nZElectrons_;
    bool byHits = false, byDeltaR = false;
    for (std::size_t t = 0; t < tracks.size(); ++t) {
      if (!byHits && particleOfTrack[t] == id) {
        byHits = true;
        ptResponse_->Fill(tracks[t].pt() / p4.pt());
      }
      if (reco::deltaR(tracks[t].eta(), tracks[t].phi(), p4.eta(), p4.phi()) < maxDeltaR_)
        byDeltaR = true;
    }
    nByHits_ += byHits;
    nByDeltaR_ += byDeltaR;
    nByBoth_ += byHits && byDeltaR;
  }
}

void GsfTrackTruthAnalyzer::endJob() {
  edm::LogPrint("GsfTrackTruthAnalyzer") << "GSF tracks " << nTracks_ << ": " << nPure_
                                         << " with a particle owning 75% of the hits, " << nElectron_
                                         << " of them an electron, " << nFromZ_ << " an electron from the Z\n"
                                         << "Z electrons " << nZElectrons_ << ": found by hits " << nByHits_
                                         << ", by delta R " << nByDeltaR_ << ", by both " << nByBoth_;
}

DEFINE_FWK_MODULE(GsfTrackTruthAnalyzer);
