#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""Which vector boson was produced together with the Higgs, and how did it decay?

    python3 vh.py ../../Common/fixtures/vh.json
    python3 vh.py step3.root -n 2

The C++ twin is cpp/Vh.cc and prints the same lines.
"""

import argparse
import sys

from TruthGraphAnalysis.Common.exampleSupport import WEAK_BOSONS, decayMode, eventNumber, graphsFrom, pt


def run(graph, out=sys.stdout):
    """Prints one line per object of interest."""
    for higgs in graph.signalParticles():
        for sibling in graph.productionSiblings(higgs):
            if abs(graph.pdgId(sibling)) in WEAK_BOSONS:
                out.write("vh: Higgs with %d pt %.2f GeV, boson decay %s\n"
                          % (graph.pdgId(sibling), pt(graph.p4(sibling)), decayMode(graph, sibling)))


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
