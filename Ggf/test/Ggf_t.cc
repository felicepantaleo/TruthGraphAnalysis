// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "TruthGraphAnalysis/Common/interface/GraphFromJson.h"
#include "TruthGraphAnalysis/Ggf/cpp/Ggf.h"

namespace {
  std::string path(std::string const& relative) {
    const char* base = std::getenv("CMSSW_BASE");
    return std::string(base != nullptr ? base : ".") + "/src/TruthGraphAnalysis/" + relative;
  }
}  // namespace

class TestGgf : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestGgf);
  CPPUNIT_TEST(testMatchesTheExpectedOutput);
  CPPUNIT_TEST_SUITE_END();

public:
  // REQUIRED: on the fixture the example prints exactly test/expected.txt, which the
  // python twin matches too, so the two languages cannot drift apart.
  void testMatchesTheExpectedOutput() {
    const truth::Graph graph = tga::graphFromJson(path("Common/fixtures/ggf.json"));
    std::ostringstream out;
    tga::ggf::run(graph, out);
    std::ifstream expectedFile(path("Ggf/test/expected.txt"));
    CPPUNIT_ASSERT_MESSAGE("test/expected.txt is missing", expectedFile.is_open());
    std::stringstream expected;
    expected << expectedFile.rdbuf();
    CPPUNIT_ASSERT_EQUAL(expected.str(), out.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestGgf);
