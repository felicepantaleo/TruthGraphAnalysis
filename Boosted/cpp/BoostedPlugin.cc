// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// Runs the Boosted example on the logical graph and the hit index of each event, through
// cmsRun.

#include <sstream>
#include <string>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/LogicalGraphHitIndex.h"
#include "TruthGraphAnalysis/Boosted/cpp/Boosted.h"

namespace {
  tga::boosted::Parent parentFromName(std::string const& name) {
    if (name == "tau") {
      return tga::boosted::Parent::Tau;
    }
    if (name == "top") {
      return tga::boosted::Parent::Top;
    }
    throw cms::Exception("Configuration") << "parent is '" << name << "', expected tau or top";
  }
}  // namespace

class TruthGraphBoostedExample : public edm::global::EDAnalyzer<> {
public:
  explicit TruthGraphBoostedExample(edm::ParameterSet const& pset)
      : graphToken_(consumes<truth::Graph>(pset.getParameter<edm::InputTag>("src"))),
        hitIndexToken_(consumes<truth::LogicalGraphHitIndex>(pset.getParameter<edm::InputTag>("hitIndex"))),
        parent_(parentFromName(pset.getParameter<std::string>("parent"))),
        cone_(pset.getParameter<double>("cone")) {}

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("src", edm::InputTag("truthLogicalGraphProducer"));
    desc.add<edm::InputTag>("hitIndex", edm::InputTag("truthLogicalGraphHitIndexProducer"))
        ->setComment("The hit index built from the graph in src.");
    desc.add<std::string>("parent", "tau")->setComment("tau or top");
    desc.add<double>("cone", tga::boosted::kCone)->setComment("The cone of the Delta R match.");
    descriptions.addWithDefaultLabel(desc);
  }

  void analyze(edm::StreamID, edm::Event const& event, edm::EventSetup const&) const override {
    std::ostringstream out;
    out << "== event " << event.id().event() << "\n";
    tga::boosted::run(event.get(graphToken_), event.get(hitIndexToken_), parent_, cone_, out);
    edm::LogPrint("BoostedExample") << out.str();
  }

private:
  const edm::EDGetTokenT<truth::Graph> graphToken_;
  const edm::EDGetTokenT<truth::LogicalGraphHitIndex> hitIndexToken_;
  const tga::boosted::Parent parent_;
  const double cone_;
};

DEFINE_FWK_MODULE(TruthGraphBoostedExample);
