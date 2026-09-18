#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""What was produced together with the top: the recoil quark, the associated W, or the b?

    python3 singletop.py ../../Common/fixtures/singletop.json
    python3 singletop.py step3.root -n 2

The C++ twin is cpp/SingleTop.cc and prints the same lines.
"""

import argparse
import sys

from TruthGraphAnalysis.Common.exampleSupport import eventNumber, graphsFrom, isParton, pt


def run(graph, out=sys.stdout):
    """Prints one line per object of interest."""
    for t in graph.signalParticles():
        for partner in graph.productionSiblings(t):
            a = abs(graph.pdgId(partner))
            kind = ("associated W" if a == 24 else "top" if a == 6 else "b" if a == 5
                    else "recoil quark" if isParton(a) else "other")
            out.write("singletop: top with %s %d pt %.2f GeV\n" % (kind, graph.pdgId(partner), pt(graph.p4(partner))))


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
