#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""What is the mass of the boson pair, and how did each boson decay?

    python3 diboson.py ../../Common/fixtures/diboson.json
    python3 diboson.py step3.root -n 2

The C++ twin is cpp/Diboson.cc and prints the same lines.
"""

import argparse
import sys

from TruthGraphAnalysis.Common.exampleSupport import add, decayMode, eventNumber, graphsFrom, mass, pt


def run(graph, out=sys.stdout):
    """Prints one line per object of interest."""
    bosons = [i for i in graph.signalParticles() if abs(graph.pdgId(i)) in (23, 24)]
    if len(bosons) >= 2:
        out.write("diboson: m(VV) %.2f GeV\n" % mass(add(graph.p4(bosons[0]), graph.p4(bosons[1]))))
    for boson in bosons:
        out.write("  %d pt %.2f GeV, decay %s\n" % (graph.pdgId(boson), pt(graph.p4(boson)), decayMode(graph, boson)))


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
