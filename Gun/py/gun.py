#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""Per gun particle: how many reconstructable products does it have, and how many of its descendants reach the calorimeter?

    python3 gun.py ../../Common/fixtures/tentau.json
    python3 gun.py step3.root -n 2

The C++ twin is cpp/Gun.cc and prints the same lines.
"""

import argparse
import sys

from TruthGraphAnalysis.Common.exampleSupport import eventNumber, graphsFrom


def run(graph, out=sys.stdout):
    """Prints one line per object of interest."""
    for seed in graph.signalParticles():
        # The gun particle and everything below it: a stable gun particle is its own product.
        subgraph = [seed] + graph.descendants(seed)
        products = sum(1 for i in subgraph if graph.isAtLevel(i, "reconstructableFromSignal"))
        atCalo = sum(1 for i in subgraph if graph.isAtLevel(i, "caloBoundary"))
        out.write("gun %d E %.2f GeV: %d reconstructable products, %d descendants reach the calorimeter\n"
                  % (graph.pdgId(seed), graph.p4(seed)[3], products, atCalo))


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
