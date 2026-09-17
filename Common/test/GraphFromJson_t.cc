// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <algorithm>
#include <cstdlib>
#include <string>

#include "TruthGraphAnalysis/Common/interface/GraphFromJson.h"

namespace {
  std::string fixture(std::string const& name) {
    const char* base = std::getenv("CMSSW_BASE");
    return std::string(base != nullptr ? base : ".") + "/src/TruthGraphAnalysis/Common/fixtures/" + name;
  }
}  // namespace

class TestGraphFromJson : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestGraphFromJson);
  CPPUNIT_TEST(testReadsAConsistentGraph);
  CPPUNIT_TEST_SUITE_END();

public:
  // REQUIRED: the fixture reads back as a consistent graph whose particles keep their
  // levels, momenta and interaction ids, and whose adjacency goes both ways.
  void testReadsAConsistentGraph() {
    const truth::Graph graph = tga::graphFromJson(fixture("zee.json"));
    CPPUNIT_ASSERT(graph.isConsistent());
    CPPUNIT_ASSERT(graph.nParticles() > 0);

    std::size_t signal = 0;
    std::size_t withMomentum = 0;
    for (uint32_t id = 0; id < graph.nParticles(); ++id) {
      auto const& data = graph.particles()[id];
      signal += data.isAtLevel(truth::LevelFlag::Signal) ? 1 : 0;
      withMomentum += data.hasMomentum() ? 1 : 0;
      for (const uint32_t vertexId : graph.decayVertices(id)) {
        const auto incoming = graph.incomingParticles(vertexId);
        CPPUNIT_ASSERT(std::find(incoming.begin(), incoming.end(), id) != incoming.end());
      }
    }
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, signal);
    CPPUNIT_ASSERT(withMomentum > 0);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestGraphFromJson);
