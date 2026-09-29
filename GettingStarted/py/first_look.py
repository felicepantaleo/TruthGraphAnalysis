#!/usr/bin/env python3
"""Prints what one event of the truth graph holds.

    python3 first_look.py step3.root -n 2
"""
import argparse

from PhysicsTools.TruthInfo.graphTools import eventGraphs

parser = argparse.ArgumentParser()
parser.add_argument("file")
parser.add_argument("-n", "--maxEvents", type=int, default=1)
args = parser.parse_args()

for graph in eventGraphs(args.file, maxEvents=args.maxEvents):
    particles = range(graph.nParticles())
    gen = sum(1 for i in particles if graph.particle(i)["hasGen"])
    sim = sum(1 for i in particles if graph.particle(i)["hasSim"])
    signal = sum(1 for i in particles if graph.isSignal(i))
    print(f"{graph.nParticles()} particles ({gen} with GEN, {sim} with SIM, {signal} signal), "
          f"{graph.nVertices()} vertices, {len(graph.interactions())} interactions")
    # The last copy of each top, and its children: a b and a W.
    for i in particles:
        if abs(graph.pdgId(i)) == 6 and graph.lastCopy(i) == i:
            print("  top", i, "->", [graph.pdgId(c) for c in graph.children(i)])
