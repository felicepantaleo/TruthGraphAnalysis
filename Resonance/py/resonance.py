#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""Which two leptons does the Z decay to, and does their invariant mass reproduce the generator mass of the Z?

    python3 resonance.py ../../Common/fixtures/zee.json
    python3 resonance.py step3.root -n 2

The C++ twin is cpp/Resonance.cc and prints the same lines.
"""

import argparse
import math
import sys

from PhysicsTools.TruthInfo.graphTools import bunchCrossingOf, eventIndexOf
from TruthGraphAnalysis.Common.exampleSupport import (LEPTONS, NEUTRINOS, add, decayMode, eta,
                                                      firstChildWithPdgId, graphsFrom, mass,
                                                      productionSiblings, pt)


def run(graph, out=sys.stdout):
    """Prints one line per object of interest."""
    for z in graph.particlesOfLevel("signal"):
        legs = [c for c in graph.children(graph.lastCopy(z)) if abs(graph.pdgId(c)) in LEPTONS]
        if len(legs) != 2:
            out.write("resonance %d: decay mode %s\n" % (graph.pdgId(z), decayMode(graph, z)))
            continue
        out.write("resonance %d -> %d %d: m(ll) %.2f GeV, generator mass %.2f GeV\n"
                  % (graph.pdgId(z), graph.pdgId(legs[0]), graph.pdgId(legs[1]),
                     mass(add(graph.p4(legs[0]), graph.p4(legs[1]))), mass(graph.p4(z))))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-n", "--maxEvents", type=int, default=-1)
    parser.add_argument("inputs", nargs="+", help="an EDM file, or one or more JSON dumps")
    args = parser.parse_args()
    for graph in graphsFrom(args.inputs, args.maxEvents):
        print("== event %s" % (graph.eventId,))
        run(graph)


if __name__ == "__main__":
    main()
