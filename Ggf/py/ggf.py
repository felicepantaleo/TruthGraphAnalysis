#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""What does the detector see of the Higgs: how many reconstructable products, and which fraction of its energy is visible?

    python3 ggf.py ../../Common/fixtures/ggf.json
    python3 ggf.py step3.root -n 2

The C++ twin is cpp/Ggf.cc and prints the same lines.
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
    for higgs in graph.particlesOfLevel("signal"):
        products = [i for i in graph.descendants(higgs) if graph.isAtLevel(i, "reconstructableFromSignal")]
        visible = sum(graph.p4(i)[3] for i in products if abs(graph.pdgId(i)) not in NEUTRINOS)
        out.write("ggf: Higgs E %.2f GeV -> %d reconstructable products, visible fraction %.3f\n"
                  % (graph.p4(higgs)[3], len(products), visible / graph.p4(higgs)[3]))


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
