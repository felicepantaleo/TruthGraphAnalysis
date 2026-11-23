// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "TruthGraphAnalysis/Boosted/cpp/Boosted.h"

namespace {
  using tga::boosted::Product;

  std::string path(std::string const& relative) {
    const char* base = std::getenv("CMSSW_BASE");
    return std::string(base != nullptr ? base : ".") + "/src/TruthGraphAnalysis/" + relative;
  }

  truth::LogicalGraphHitIndex::Hit hit(uint32_t detId, float energy) {
    return {detId, truth::LogicalGraphHitIndex::Hit::kInvalidRecHitIndex, energy};
  }
}  // namespace

class TestBoosted : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestBoosted);
  CPPUNIT_TEST(testMatchesTheExpectedOutput);
  CPPUNIT_TEST_SUITE_END();

public:
  // REQUIRED: on the hit lists below the hit logic prints exactly test/expected.txt, which
  // the python twin matches too. The numbers, by hand, with the cone of 0.4:
  //
  // tau 15. The pi- has 2.0 (two hits), 1.0, 0.5, 0.5 GeV in the cells 101 to 104. The pi0 has
  // 1.5, 0.5, 3.0, 1.0 GeV in the cells 103 to 106. The cells 103 and 104 are shared: 2 of 6
  // cells, 2.0 + 1.0 = 3.0 of 10.0 GeV, fraction 0.30. Purity of the pi-:
  // 4.0 / (2.0 + 1.0 + 2.0 + 1.0) = 0.67. Purity of the pi0: 6.0 / (2.0 + 1.0 + 3.0 + 1.0)
  // = 0.86. The axes are 0.10 apart: the Delta R label is lost twice, the hit label never.
  //
  // top 6, boosted. The b has 4.0 and 3.0 GeV in the cells 201 and 202, the W has 0.5 and
  // 0.25 GeV in 202 and 203. The cell 202 is shared: 1 of 3 cells, 3.5 of 7.75 GeV, fraction
  // 0.45. Purities: 7.0 / 7.5 = 0.93 and 0.75 / 3.75 = 0.20, so the W loses its hit label.
  // The axes are 0.30 apart: the Delta R label is lost twice.
  //
  // top -6, resolved. The b has 2.0 GeV in the cell 301, the W has 1.0 GeV in 302, and the
  // axes are 1.00 apart. No cell is shared, and both labels are kept twice.
  //
  // tau -15. One electron with no hit: no other product, so the Delta R label is kept, and no
  // hit, so the hit label is lost.
  void testMatchesTheExpectedOutput() {
    const std::vector<Product> tau{
        {-211, 0.10, 0.20, {hit(101, 1.5f), hit(101, 0.5f), hit(102, 1.f), hit(103, 0.5f), hit(104, 0.5f)}},
        {111, 0.10, 0.30, {hit(103, 1.5f), hit(104, 0.5f), hit(105, 3.f), hit(106, 1.f)}}};
    const std::vector<Product> boostedTop{{5, 0.00, 0.00, {hit(201, 4.f), hit(202, 3.f)}},
                                          {24, 0.00, 0.30, {hit(202, 0.5f), hit(203, 0.25f)}}};
    const std::vector<Product> resolvedTop{{-5, 1.00, 0.00, {hit(301, 2.f)}}, {-24, 0.00, 0.00, {hit(302, 1.f)}}};
    const std::vector<Product> leptonicTau{{11, 2.00, 1.00, {}}};

    std::ostringstream out;
    tga::boosted::report(15, tau, tga::boosted::kCone, out);
    tga::boosted::report(6, boostedTop, tga::boosted::kCone, out);
    tga::boosted::report(-6, resolvedTop, tga::boosted::kCone, out);
    tga::boosted::report(-15, leptonicTau, tga::boosted::kCone, out);

    std::ifstream expectedFile(path("Boosted/test/expected.txt"));
    CPPUNIT_ASSERT_MESSAGE("test/expected.txt is missing", expectedFile.is_open());
    std::stringstream expected;
    expected << expectedFile.rdbuf();
    CPPUNIT_ASSERT_EQUAL(expected.str(), out.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestBoosted);
