#!/usr/bin/env python3
"""Mass of the two leptons of the Z, from the generator record in the graph.

    python3 z_mass.py step3.root -n 1000 -o z_mass.png
"""
import argparse
import math

from PhysicsTools.TruthInfo.graphTools import eventGraphs

parser = argparse.ArgumentParser()
parser.add_argument("file")
parser.add_argument("-n", "--maxEvents", type=int, default=-1)
parser.add_argument("-o", "--output", default="z_mass.png")
args = parser.parse_args()

masses = []
for graph in eventGraphs(args.file, maxEvents=args.maxEvents):
    for z in range(graph.nParticles()):
        if graph.pdgId(z) != 23 or graph.lastCopy(z) != z:
            continue
        leptons = [c for c in graph.children(z) if abs(graph.pdgId(c)) in (11, 13, 15)]
        if len(leptons) != 2:
            continue
        e, px, py, pz = (sum(graph.p4(l)[k] for l in leptons) for k in (3, 0, 1, 2))
        masses.append(math.sqrt(max(e * e - px * px - py * py - pz * pz, 0.)))

print(f"{len(masses)} Z decays, mean mass {sum(masses) / len(masses):.2f} GeV")
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
plt.hist(masses, bins=60, range=(60, 120))
plt.xlabel("m(ll) [GeV]")
plt.ylabel("events")
plt.savefig(args.output)
