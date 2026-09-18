#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""What are the dijet mass and the rapidity gap of the two quarks that recoil against the Higgs?

    python3 vbf.py ../../Common/fixtures/vbf.json
    python3 vbf.py step3.root -n 2

The C++ twin is cpp/Vbf.cc and prints the same lines.
"""

import argparse
import sys

from TruthGraphAnalysis.Common.exampleSupport import add, eta, eventNumber, graphsFrom, isParton, mass, pt


def run(graph, out=sys.stdout):
    """Prints one line per object of interest."""
    higgs = graph.signalParticles()
    tagging = [s for s in graph.productionSiblings(higgs[0]) if isParton(graph.pdgId(s))] if higgs else []
    if len(tagging) < 2:
        out.write("vbf: %d Higgs, %d tagging partons\n" % (len(higgs), len(tagging)))
        return
    tagging.sort(key=lambda i: -pt(graph.p4(i)))
    j1, j2 = tagging[0], tagging[1]
    out.write("vbf: %d Higgs, tagging partons %d %d: m(jj) %.2f GeV, |delta eta| %.2f\n"
              % (len(higgs), graph.pdgId(j1), graph.pdgId(j2), mass(add(graph.p4(j1), graph.p4(j2))),
                 abs(eta(graph.p4(j1)) - eta(graph.p4(j2)))))


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
