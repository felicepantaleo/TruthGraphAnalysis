// Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

#include "TruthGraphAnalysis/Common/interface/GraphFromJson.h"

#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "PhysicsTools/TruthInfo/interface/TruthLevels.h"

namespace {

  void buildCSR(uint32_t nSources,
                std::vector<std::pair<uint32_t, uint32_t>>& pairs,
                std::vector<uint32_t>& offsets,
                std::vector<uint32_t>& flat) {
    std::sort(pairs.begin(), pairs.end());
    pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
    offsets.assign(nSources + 1, 0);
    for (auto const& pair : pairs) {
      ++offsets[pair.first + 1];
    }
    for (uint32_t i = 1; i <= nSources; ++i) {
      offsets[i] += offsets[i - 1];
    }
    flat.assign(pairs.size(), 0);
    auto cursor = offsets;
    for (auto const& pair : pairs) {
      flat[cursor[pair.first]++] = pair.second;
    }
  }

  bool isKnownLevel(std::string const& name) {
    for (auto const& row : truth::kLevelTable) {
      if (name == row.name) {
        return true;
      }
    }
    return false;
  }

  // The dumper writes null for a value that is not a number; graphTools reads it as 0.
  double numberOr0(nlohmann::json const& value) { return value.is_null() ? 0. : value.get<double>(); }

  // The names come from truth::particleRoleName, so a renamed role still matches; an
  // unknown one throws, as roleOf does for a vertex.
  uint8_t particleRoleOf(std::string const& name) {
    for (const auto role :
         {truth::ParticleRole::Normal, truth::ParticleRole::Connector, truth::ParticleRole::SignalStandIn}) {
      if (name == truth::particleRoleName(role)) {
        return static_cast<uint8_t>(role);
      }
    }
    throw std::runtime_error("unknown particle role " + name);
  }

  uint32_t levelFlagsOf(nlohmann::json const& names) {
    uint32_t flags = 0;
    for (auto const& name : names) {
      const std::string level = name.get<std::string>();
      if (level == truth::kSignalLevelName) {
        flags |= static_cast<uint32_t>(truth::LevelFlag::Signal);
        continue;
      }
      // A level this build does not know is skipped, as graphTools skips it, so a dump
      // written by a newer release still reads.
      if (!isKnownLevel(level)) {
        continue;
      }
      flags |= static_cast<uint32_t>(truth::levelFlagOf(truth::levelFromName(level)));
    }
    return flags;
  }

  uint8_t roleOf(std::string const& name) {
    for (int role = 0; role < truth::kVertexRoleCount; ++role) {
      if (name == truth::vertexRoleName(static_cast<truth::VertexRole>(role))) {
        return static_cast<uint8_t>(role);
      }
    }
    throw std::runtime_error("unknown vertex role " + name);
  }

  uint8_t reasonOf(std::string const& name) {
    for (int reason = 0; reason < truth::kVertexReasonCount; ++reason) {
      if (name == truth::vertexReasonName(static_cast<truth::VertexReason>(reason))) {
        return static_cast<uint8_t>(reason);
      }
    }
    throw std::runtime_error("unknown vertex reason " + name);
  }

}  // namespace

namespace tga {

  truth::Graph graphFromJson(std::string const& path) {
    std::ifstream in(path);
    if (!in) {
      throw std::runtime_error("cannot open " + path);
    }
    const nlohmann::json payload = nlohmann::json::parse(in);

    truth::Graph graph;
    // What the level rules read from the graph itself; an older dump has none.
    for (auto const& [key, target] : {std::pair{"signalSeedPdgIds", &graph.signalSeedPdgIds()},
                                      std::pair{"seedHadronFlavors", &graph.seedHadronFlavors()},
                                      std::pair{"reconstructablePdgIds", &graph.reconstructablePdgIds()}}) {
      if (payload.contains(key)) {
        *target = payload.at(key).get<std::vector<int32_t>>();
      }
    }
    auto& particles = graph.particles();
    particles.resize(payload.at("particles").size());
    for (auto const& item : payload.at("particles")) {
      auto& data = particles.at(item.at("id").get<uint32_t>());
      data.pdgId = item.at("pdgId").get<int32_t>();
      data.status = item.at("status").get<int16_t>();
      data.statusFlags = item.at("statusFlags").get<uint16_t>();
      data.genNode = item.at("hasGen").get<bool>() ? 0 : -1;
      data.simNode = item.at("hasSim").get<bool>() ? 0 : -1;
      data.eventId = item.at("eventId").get<uint64_t>();
      data.genEvent = item.at("genEvent").get<int32_t>();
      data.levelFlags = levelFlagsOf(item.at("levels"));
      data.role = particleRoleOf(item.at("role").get<std::string>());
      auto const& p4 = item.at("p4");
      data.momentum =
          math::XYZTLorentzVectorD(numberOr0(p4.at(0)), numberOr0(p4.at(1)), numberOr0(p4.at(2)), numberOr0(p4.at(3)));
    }

    auto& vertices = graph.vertices();
    vertices.resize(payload.at("vertices").size());
    std::vector<std::pair<uint32_t, uint32_t>> decay, production, incoming, outgoing;
    for (auto const& item : payload.at("vertices")) {
      const uint32_t id = item.at("id").get<uint32_t>();
      auto& data = vertices.at(id);
      data.genNode = item.at("hasGen").get<bool>() ? 0 : -1;
      data.simNode = item.at("hasSim").get<bool>() ? 0 : -1;
      data.eventId = item.at("eventId").get<uint64_t>();
      data.role = roleOf(item.at("role").get<std::string>());
      data.reason = reasonOf(item.at("reason").get<std::string>());
      auto const& x4 = item.at("x4");
      data.position =
          math::XYZTLorentzVectorD(numberOr0(x4.at(0)), numberOr0(x4.at(1)), numberOr0(x4.at(2)), numberOr0(x4.at(3)));
      for (auto const& particle : item.at("in")) {
        decay.emplace_back(particle.get<uint32_t>(), id);
        incoming.emplace_back(id, particle.get<uint32_t>());
      }
      for (auto const& particle : item.at("out")) {
        production.emplace_back(particle.get<uint32_t>(), id);
        outgoing.emplace_back(id, particle.get<uint32_t>());
      }
    }

    buildCSR(graph.nParticles(), decay, graph.particleToDecayVertexOffsets(), graph.particleToDecayVertices());
    buildCSR(graph.nParticles(),
             production,
             graph.particleToProductionVertexOffsets(),
             graph.particleToProductionVertices());
    buildCSR(graph.nVertices(), outgoing, graph.vertexToOutgoingParticleOffsets(), graph.vertexToOutgoingParticles());
    buildCSR(graph.nVertices(), incoming, graph.vertexToIncomingParticleOffsets(), graph.vertexToIncomingParticles());
    if (!graph.isConsistent()) {
      throw std::runtime_error("the graph read from " + path + " is not consistent");
    }
    return graph;
  }

}  // namespace tga
