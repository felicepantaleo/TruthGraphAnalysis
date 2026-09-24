// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <cmath>
#include <cstdlib>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "DataFormats/ForwardDetId/interface/BTLDetId.h"
#include "DataFormats/ForwardDetId/interface/ETLDetId.h"
#include "TruthGraphAnalysis/Common/interface/GraphFromJson.h"
#include "TruthGraphAnalysis/Mtd/cpp/MtdTruth.h"

namespace {
  using Hit = truth::LogicalGraphHitIndex::Hit;

  std::string path(std::string const& relative) {
    const char* base = std::getenv("CMSSW_BASE");
    return std::string(base != nullptr ? base : ".") + "/src/TruthGraphAnalysis/" + relative;
  }

  // The first particle with this pdgId and status whose momentum passes the pt cut.
  std::optional<uint32_t> find(truth::Graph const& graph, int pdgId, double ptMin) {
    for (uint32_t id = 0; id < graph.particles().size(); ++id) {
      auto const& p = graph.particles()[id];
      if (p.pdgId == pdgId && p.status == 1 && p.momentum.pt() > ptMin && std::abs(p.momentum.eta()) < 4.)
        return id;
    }
    return std::nullopt;
  }
}  // namespace

class TestMtd : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestMtd);
  CPPUNIT_TEST(testDepositsGroupByModuleAndCategory);
  CPPUNIT_TEST(testMatchNeedsACellATimeAndInBtlAnEnergy);
  CPPUNIT_TEST(testSelectionNeedsCharge);
  CPPUNIT_TEST_SUITE_END();

public:
  // REQUIRED: one deposit per (module, category), with the summed energy and the
  // energy-weighted time, as a MtdSimLayerCluster; an ETL deposit knows its disc.
  void testDepositsGroupByModuleAndCategory() {
    const uint32_t btl = BTLDetId(1, 3, 1, 1, 1, 0).rawId();
    ETLDetId etl(1, 1, 0, 1, 1, 1, 1, 1, 1, 1);
    if (etl.nDisc() != 2)
      etl = ETLDetId(1, 2, 0, 1, 1, 1, 1, 1, 1, 1);
    CPPUNIT_ASSERT_EQUAL(2, etl.nDisc());
    const std::vector<Hit> hits{
        Hit{btl, 3u, 1.f}, Hit{btl, 4u, 3.f}, Hit{btl, 3u << 24 | 3u, 2.f}, Hit{etl.rawId(), 1u << 16 | 2u, 1.f}};
    const std::vector<float> times{1.f, 3.f, 7.f, 4.f};
    const auto deposits = tga::mtd::deposits(hits, times);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), deposits.size());
    for (auto const& d : deposits) {
      if (d.module == btl && d.category == 0) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4., d.energy, 1e-6);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.5, d.time, 1e-6);
        CPPUNIT_ASSERT(d.cells == (std::vector<uint32_t>{3u, 4u}));
      } else if (d.module == btl) {
        CPPUNIT_ASSERT_EQUAL(uint32_t(3), d.category);
        CPPUNIT_ASSERT(d.cells == std::vector<uint32_t>{3u});
      } else {
        CPPUNIT_ASSERT_EQUAL(2, d.etlDisc());
        CPPUNIT_ASSERT(!d.isBarrel());
      }
    }
    // Without one time per hit there is no deposit.
    CPPUNIT_ASSERT(tga::mtd::deposits(hits, std::span<const float>(times.data(), 2)).empty());
  }

  // REQUIRED: the legacy reco-to-sim rule. A shared cell and a time pull below the cut
  // match; in BTL the reco over sim energy must also stay below the cut; ETL ignores it.
  void testMatchNeedsACellATimeAndInBtlAnEnergy() {
    tga::mtd::Deposit btl;
    btl.module = BTLDetId(1, 3, 1, 1, 1, 0).rawId();
    btl.energy = 0.004;
    btl.time = 1.0;
    btl.cells = {3u, 4u};
    const tga::mtd::ClusterMatchCuts cuts;
    const std::vector<uint32_t> shared{4u, 9u}, disjoint{5u};
    CPPUNIT_ASSERT(tga::mtd::matches(btl, btl.module, shared, 0.010, 1.1, 0.03, cuts));
    CPPUNIT_ASSERT(!tga::mtd::matches(btl, btl.module, disjoint, 0.010, 1.1, 0.03, cuts));
    CPPUNIT_ASSERT(!tga::mtd::matches(btl, btl.module + 1, shared, 0.010, 1.1, 0.03, cuts));
    CPPUNIT_ASSERT(!tga::mtd::matches(btl, btl.module, shared, 0.030, 1.1, 0.03, cuts));  // energy
    CPPUNIT_ASSERT(!tga::mtd::matches(btl, btl.module, shared, 0.010, 1.5, 0.03, cuts));  // time

    tga::mtd::Deposit etl = btl;
    etl.module = ETLDetId(1, 1, 0, 1, 1, 1, 1, 1, 1, 1).rawId();
    CPPUNIT_ASSERT(tga::mtd::matches(etl, etl.module, shared, 0.030, 1.1, 0.03, cuts));
  }

  // REQUIRED: the selection keeps a charged final-state pion and rejects a photon with
  // the same kinematics, as trkTPSelAll does on the TrackingParticle charge.
  void testSelectionNeedsCharge() {
    const truth::Graph graph = tga::graphFromJson(path("Common/fixtures/top.json"));
    const tga::mtd::TruthSelection cuts;
    const auto pion = find(graph, 211, cuts.ptMin);
    const auto photon = find(graph, 22, cuts.ptMin);
    CPPUNIT_ASSERT(pion && photon);
    CPPUNIT_ASSERT(tga::mtd::passesSelection(graph, *pion, cuts));
    CPPUNIT_ASSERT(tga::mtd::isGeneratorFinalState(graph, *pion));
    CPPUNIT_ASSERT(!tga::mtd::passesSelection(graph, *photon, cuts));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMtd);
