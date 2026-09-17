#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""For each top: is there a b, how did the W decay, how large is its subgraph; and what class is the event, all hadronic, semileptonic or dilepton? A W to tau nu counts as leptonic.

    python3 top.py ../../Common/fixtures/top.json
    python3 top.py step3.root -n 2

The C++ twin is cpp/Top.cc and prints the same lines.
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
    leptonic = 0
    for t in graph.particlesOfLevel("signal"):
        b = firstChildWithPdgId(graph, t, (5,))
        w = firstChildWithPdgId(graph, t, (24,))
        mode = decayMode(graph, w) if w is not None else "none"
        leptonic += 1 if mode == "leptonic" else 0
        out.write("top %d: b %s, W %s, %d descendants\n"
                  % (graph.pdgId(t), "yes" if b is not None else "no", mode, len(graph.descendants(t))))
    out.write("top event class: %s\n" % {0: "all hadronic", 1: "semileptonic"}.get(leptonic, "dilepton"))


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
