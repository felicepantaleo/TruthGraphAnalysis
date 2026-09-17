// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "TruthGraphAnalysis/Common/interface/GraphFromJson.h"
#include "TruthGraphAnalysis/Full/cpp/Full.h"

namespace {
  std::string path(std::string const& relative) {
    const char* base = std::getenv("CMSSW_BASE");
    return std::string(base != nullptr ? base : ".") + "/src/TruthGraphAnalysis/" + relative;
  }
}  // namespace

class TestFull : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestFull);
  CPPUNIT_TEST(testMatchesTheExpectedOutput);
  CPPUNIT_TEST_SUITE_END();

public:
  // REQUIRED: on the fixture the example prints exactly test/expected.txt, which the
  // python twin matches too, so the two languages cannot drift apart.
  void testMatchesTheExpectedOutput() {
    const truth::Graph graph = tga::graphFromJson(path("Common/fixtures/top.json"));
    std::ostringstream out;
    tga::full::run(graph, out);
    std::ifstream expectedFile(path("Full/test/expected.txt"));
    std::stringstream expected;
    expected << expectedFile.rdbuf();
    CPPUNIT_ASSERT_EQUAL(expected.str(), out.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFull);
