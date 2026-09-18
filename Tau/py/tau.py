#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""How did each tau decay: how many charged hadrons and neutral pions, which decay mode
tau reconstruction would call it, and how much of the tau energy is visible?

    python3 tau.py ../../Common/fixtures/tentau.json
    python3 tau.py step3.root -n 2

The C++ twin is cpp/Tau.cc and prints the same lines.
"""

import argparse
import sys

from TruthGraphAnalysis.Common.exampleSupport import NEUTRINOS, add, eventNumber, graphsFrom

PRONGS = (211, 321)  # charged pions and kaons
NEUTRAL_KAONS = (130, 310)  # neutral, so not prongs


def collect(graph, particle, content):
    """Follows the generator decay chain down to what leaves it, as cpp/Tau.cc: through an
    intermediate resonance, stopping at a pi0, a neutral kaon, or a particle the generator
    did not decay. Only generator decay vertices are followed."""
    a = abs(graph.pdgId(particle))
    if a in NEUTRINOS:
        return
    if a == 111 or a in NEUTRAL_KAONS:
        content["pi0s" if a == 111 else "neutralKaons"] += 1
        content["visible"] = add(content["visible"], graph.p4(particle))
        return
    decayed = False
    for vertex in graph.decayVertices(particle):
        if not graph.vertex(vertex)["hasGen"]:
            continue
        decayed = True
        for child in graph.outgoingParticles(vertex):
            if child != particle:
                collect(graph, child, content)
    if decayed:
        return
    if a in PRONGS:
        content["prongs"] += 1
    elif a == 22:
        content["photons"] += 1
    elif a == 11:
        content["electrons"] += 1
    elif a == 13:
        content["muons"] += 1
    else:
        content["others"] += 1
    content["visible"] = add(content["visible"], graph.p4(particle))


def tauDecay(graph, tau):
    """Leptonic when an electron or a muon is among the generator children, as
    truth::tauDecay."""
    for vertex in graph.decayVertices(tau):
        if not graph.vertex(vertex)["hasGen"]:
            continue
        if any(abs(graph.pdgId(c)) in (11, 13) for c in graph.outgoingParticles(vertex) if c != tau):
            return "leptonic"
    return "hadronic"


def modeName(content, decay):
    if decay == "leptonic":
        return "electron" if content["electrons"] > 0 else "muon"
    name = "%dprong%dpi0" % (content["prongs"], content["pi0s"])
    if content["neutralKaons"] > 0:
        name += "%dK0" % content["neutralKaons"]
    return name


def recoDecayMode(content, decay):
    """reco::PFTau::hadronicDecayMode, as cpp/Tau.cc."""
    if decay != "hadronic":
        return -1
    if content["prongs"] < 1 or content["prongs"] > 3:
        return 15
    return 5 * (content["prongs"] - 1) + min(content["pi0s"], 4)


def run(graph, out=sys.stdout):
    """Prints one line per physical tau, then the count of each decay mode."""
    modes = {}
    for tau in graph.particlesAtLevels(["tauVisibleHadronic", "tauVisibleLeptonic"]):
        decay = tauDecay(graph, tau)
        content = dict(prongs=0, pi0s=0, neutralKaons=0, photons=0, electrons=0, muons=0, others=0,
                       visible=(0.0, 0.0, 0.0, 0.0))
        collect(graph, tau, content)
        name = modeName(content, decay)
        modes[name] = modes.get(name, 0) + 1
        energy = graph.p4(tau)[3]
        line = "tau %d E %.2f GeV: %s, reco mode %d, %d prongs %d pi0 %d photons" % (
            graph.pdgId(tau), energy, name, recoDecayMode(content, decay), content["prongs"], content["pi0s"],
            content["photons"])
        if content["others"] > 0:
            line += " %d others" % content["others"]
        out.write("%s, visible energy %.2f\n" % (line, content["visible"][3] / energy))
    for name in sorted(modes):
        out.write("mode %s: %d\n" % (name, modes[name]))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-n", "--maxEvents", type=int, default=-1)
    parser.add_argument("inputs", nargs="+", help="an EDM file, or one or more JSON dumps")
    args = parser.parse_args()
    for graph in graphsFrom(args.inputs, args.maxEvents):
        print("== event %s" % eventNumber(graph))
        run(graph)


if __name__ == "__main__":
    main()
