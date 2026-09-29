#!/usr/bin/env python3
"""Lists the visible decay products of each tau. The products are the generator-stable
descendants: hasGen drops the electrons and photons that Geant4 adds later.

    python3 tau_products.py step3.root -n 1
"""
import argparse

from PhysicsTools.TruthInfo.graphTools import eventGraphs

NEUTRINOS = (12, 14, 16)

parser = argparse.ArgumentParser()
parser.add_argument("file")
parser.add_argument("-n", "--maxEvents", type=int, default=1)
args = parser.parse_args()

for graph in eventGraphs(args.file, maxEvents=args.maxEvents):
    for tau in range(graph.nParticles()):
        if abs(graph.pdgId(tau)) != 15 or graph.lastCopy(tau) != tau:
            continue
        products = [d for d in graph.descendants(tau)
                    if graph.particle(d)["hasGen"] and graph.particle(d)["status"] == 1
                    and abs(graph.pdgId(d)) not in NEUTRINOS]
        allDescendants = len(graph.descendants(tau))
        print(f"tau {tau}: {sorted(graph.pdgId(d) for d in products)} "
              f"({allDescendants} descendants in total, Geant4 included)")
