#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""What does the whole event hold, interaction by interaction, and how large is the reconstructable final state of the signal and of the pile-up?

    python3 full.py ../../Common/fixtures/top.json
    python3 full.py step3.root -n 2

The C++ twin is cpp/Full.cc and prints the same lines.
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
    for eventId in graph.interactions():
        out.write("full: interaction bx %d index %d: %d particles\n"
                  % (bunchCrossingOf(eventId), eventIndexOf(eventId), len(graph.particlesOfInteraction(eventId))))
    for side, keep in (("signal", graph.isSignal), ("pileup", graph.isFromPileup)):
        members = [i for i in graph.particlesOfLevel("reconstructableFinalState") if keep(i)]
        energy = sum(graph.p4(i)[3] for i in members)
        out.write("full: %s reconstructable final state %d objects, %.1f GeV, %d without momentum\n"
                  % (side, len(members), energy, sum(1 for i in members if not graph.hasMomentum(i))))


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
