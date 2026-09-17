// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>
//
// Runs the Resonance example on the logical graph of each event, through cmsRun.

#include <sstream>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "TruthGraphAnalysis/Resonance/cpp/Resonance.h"

class TruthGraphResonanceExample : public edm::global::EDAnalyzer<> {
public:
  explicit TruthGraphResonanceExample(edm::ParameterSet const& pset)
      : token_(consumes<truth::Graph>(pset.getParameter<edm::InputTag>("src"))) {}

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("src", edm::InputTag("truthLogicalGraphProducer"));
    descriptions.addWithDefaultLabel(desc);
  }

  void analyze(edm::StreamID, edm::Event const& event, edm::EventSetup const&) const override {
    std::ostringstream out;
    out << "== event " << event.id().event() << "\n";
    tga::resonance::run(event.get(token_), out);
    edm::LogPrint("ResonanceExample") << out.str();
  }

private:
  const edm::EDGetTokenT<truth::Graph> token_;
};

DEFINE_FWK_MODULE(TruthGraphResonanceExample);
