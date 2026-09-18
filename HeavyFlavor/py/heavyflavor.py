#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""How far does each b hadron fly before it decays, and how many charm hadrons does it leave below it?

    python3 heavyflavor.py ../../Common/fixtures/top.json
    python3 heavyflavor.py step3.root -n 2

The C++ twin is cpp/HeavyFlavor.cc and prints the same lines.
"""

import argparse
import math
import sys

from TruthGraphAnalysis.Common.exampleSupport import eventNumber, graphsFrom, pt


def run(graph, out=sys.stdout):
    """Prints one line per object of interest."""
    for hadron in graph.particlesOfLevel("bHadrons"):
        production = graph.productionVertices(hadron)
        decay = graph.decayVertices(hadron)
        flight = "n/a"
        if production and decay:
            x0, x1 = graph.vertex(production[0])["x4"], graph.vertex(decay[0])["x4"]
            flight = "%.3f cm" % math.sqrt(sum((a - b) ** 2 for a, b in zip(x0[:3], x1[:3])))
        charm = sum(1 for i in graph.descendants(hadron) if graph.isAtLevel(i, "cHadrons"))
        out.write("heavyflavor: %d pt %.2f GeV, flight %s, %d charm hadron%s below\n"
                  % (graph.pdgId(hadron), pt(graph.p4(hadron)), flight, charm, "" if charm == 1 else "s"))


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
