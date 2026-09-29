#!/usr/bin/env python3
"""Counts the interactions of a pileup event and the particles of each.

    python3 pileup.py step3.root -n 1
"""
import argparse
from collections import Counter

from PhysicsTools.TruthInfo.graphTools import eventGraphs

parser = argparse.ArgumentParser()
parser.add_argument("file")
parser.add_argument("-n", "--maxEvents", type=int, default=1)
args = parser.parse_args()

for graph in eventGraphs(args.file, maxEvents=args.maxEvents):
    particles = range(graph.nParticles())
    perCrossing = Counter(graph.bunchCrossing(i) for i in particles)
    signal = sum(1 for i in particles if graph.isSignal(i))
    print(f"{graph.nParticles()} particles, {signal} signal, {len(graph.interactionIds())} interactions")
    print("  particles per bunch crossing:", dict(sorted(perCrossing.items())))
    # The same question at a truth level: the particles that reach the calorimeter.
    boundary = graph.particlesOfLevel("caloBoundary")
    print(f"  caloBoundary: {len(boundary)} particles, "
          f"{sum(1 for i in boundary if graph.isSignal(i))} signal")
