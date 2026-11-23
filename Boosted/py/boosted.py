#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""In a boosted tau or top the footprints of the decay products overlap. Which detector
cells do the products share, and does each product keep its label: from the hits, and from
a Delta R match?

    python3 boosted.py step3.root -n 2
    python3 boosted.py step3.root -n 2 --parent top

The C++ twin is cpp/Boosted.cc and prints the same lines.
"""

import argparse
import math
import sys

from TruthGraphAnalysis.Common.exampleSupport import NEUTRINOS, eta

CONE = 0.4  # the cone of the Delta R match: the radius of an AK4 jet
MINIMUM_PURITY = 0.5


def deltaR(eta1, phi1, eta2, phi2):
    """reco::deltaR."""
    dphi = phi1 - phi2
    if abs(dphi) > math.pi:
        dphi -= round(dphi * (1.0 / (2.0 * math.pi))) * (2.0 * math.pi)
    deta = eta1 - eta2
    return math.sqrt(deta * deta + dphi * dphi)


def report(parentPdgId, products, cone=CONE, out=sys.stdout):
    """The hit logic, with no graph. A product is a dict with pdgId, eta, phi and hits, and
    a hit is (detId, energy); a cell can repeat in the list. Prints one line for the
    parent, one line per product and the count of the labels kept, as cpp/Boosted.cc.

    shared cell      a cell with energy from two or more products
    purity           the energy of the product over the energy of all products in its cells
    hit label        kept when the purity is above 0.5
    Delta R label    kept when no other product axis is inside the cone
    """
    n = len(products)
    # The energy of each product in each cell. A hit with no energy is no hit.
    cells = {}
    for i, product in enumerate(products):
        for detId, energy in product["hits"]:
            if energy > 0.0:
                cells.setdefault(detId, [0.0] * n)[i] += energy

    everything = dict(cells=0, energy=0.0, sharedCells=0, sharedEnergy=0.0)
    own = [dict(cells=0, energy=0.0, sharedCells=0, sharedEnergy=0.0, cellEnergy=0.0) for _ in products]
    for detId in sorted(cells):
        energies = cells[detId]
        cellEnergy = 0.0
        owners = 0
        for energy in energies:
            cellEnergy += energy
            owners += 1 if energy > 0.0 else 0
        shared = owners > 1
        everything["cells"] += 1
        everything["energy"] += cellEnergy
        if shared:
            everything["sharedCells"] += 1
            everything["sharedEnergy"] += cellEnergy
        for i in range(n):
            if energies[i] <= 0.0:
                continue
            own[i]["cells"] += 1
            own[i]["energy"] += energies[i]
            own[i]["cellEnergy"] += cellEnergy
            if shared:
                own[i]["sharedCells"] += 1
                own[i]["sharedEnergy"] += energies[i]

    fraction = everything["sharedEnergy"] / everything["energy"] if everything["energy"] > 0.0 else 0.0
    out.write("parent %d: %d products, %d cells %.2f GeV, %d shared cells %.2f GeV, shared energy fraction %.2f\n" % (
        parentPdgId, n, everything["cells"], everything["energy"], everything["sharedCells"],
        everything["sharedEnergy"], fraction))

    keptByDeltaR = 0
    keptByHits = 0
    for i, product in enumerate(products):
        purity = own[i]["energy"] / own[i]["cellEnergy"] if own[i]["cellEnergy"] > 0.0 else 0.0
        hitLabel = purity > MINIMUM_PURITY

        # The nearest other product: a Delta R match cannot tell two axes in one cone apart.
        nearest = None
        nearestDeltaR = 0.0
        for j, other in enumerate(products):
            if j == i:
                continue
            dR = deltaR(product["eta"], product["phi"], other["eta"], other["phi"])
            if nearest is None or dR < nearestDeltaR:
                nearest = j
                nearestDeltaR = dR
        deltaRLabel = nearest is None or nearestDeltaR >= cone
        keptByDeltaR += 1 if deltaRLabel else 0
        keptByHits += 1 if hitLabel else 0

        line = "  product %d: %d cells %.2f GeV, %d shared %.2f GeV, purity %.2f, " % (
            product["pdgId"], own[i]["cells"], own[i]["energy"], own[i]["sharedCells"], own[i]["sharedEnergy"], purity)
        if nearest is None:
            line += "no other product"
        else:
            line += "nearest product %d at dR %.2f" % (products[nearest]["pdgId"], nearestDeltaR)
        out.write("%s, Delta R label %s, hit label %s, same label %s\n" % (
            line, "kept" if deltaRLabel else "lost", "kept" if hitLabel else "lost",
            "yes" if deltaRLabel == hitLabel else "no"))
    out.write("  labels kept: Delta R %d of %d, hits %d of %d\n" % (keptByDeltaR, n, keptByHits, n))


def decayProducts(graph, particle):
    """The visible decay products: what leaves the generator decay vertices of the last
    copy. A top gives a b and a W. The W stays one product: its two quarks hadronize
    together, so the graph gives the same hadrons to both."""
    last = graph.lastCopy(particle)
    products = []
    for vertex in graph.decayVertices(last):
        if not graph.vertex(vertex)["hasGen"]:
            continue
        for child in graph.outgoingParticles(vertex):
            if child != last and abs(graph.pdgId(child)) not in NEUTRINOS:
                products.append(child)
    return products


def parents(graph, parent):
    if parent == "tau":
        return graph.particlesAtLevels(["tauVisibleHadronic", "tauVisibleLeptonic"])
    # A top is a member of no level unless the graph has the top preset, so take the last
    # copy of each top.
    return [p for p in range(graph.nParticles()) if abs(graph.pdgId(p)) == 6 and graph.lastCopy(p) == p]


def run(graph, subgraphHits, parent="tau", cone=CONE, out=sys.stdout):
    """Calls report for each tau of the two tau levels, or for the last copy of each top.
    subgraphHits(particle) gives the calorimeter hits of a particle and of its descendants
    as (detId, energy): a pi0, a quark or a W has no hit of its own."""
    for particle in parents(graph, parent):
        products = []
        for child in decayProducts(graph, particle):
            p4 = graph.p4(child)
            products.append(dict(pdgId=graph.pdgId(child), eta=eta(p4), phi=math.atan2(p4[1], p4[0]),
                                 hits=subgraphHits(child)))
        report(graph.pdgId(particle), products, cone, out)


def events(fileName, maxEvents=-1):
    """Yields the event number, the graph and the subgraphHits function of each event of
    an EDM file, through FWLite."""
    import ROOT

    ROOT.gROOT.SetBatch(True)
    ROOT.gSystem.Load("libFWCoreFWLite.so")
    ROOT.FWLiteEnabler.enable()
    from DataFormats.FWLite import Events, Handle
    from PhysicsTools.TruthInfo.graphTools import TruthGraphView

    graphHandle = Handle("truth::Graph")
    indexHandle = Handle("truth::LogicalGraphHitIndex")
    for i, event in enumerate(Events(fileName)):
        if 0 <= maxEvents <= i:
            break
        event.getByLabel("truthLogicalGraphProducer", graphHandle)
        event.getByLabel("truthLogicalGraphHitIndexProducer", indexHandle)
        index = indexHandle.product()

        def subgraphHits(particle, index=index):
            hits = ROOT.std.vector("truth::LogicalGraphHitIndex::Hit")()
            index.appendSubgraphHits(ROOT.truth.HitChannel.Calo, particle, hits)
            return [(hit.detId, hit.energy) for hit in hits]

        yield event.eventAuxiliary().event(), TruthGraphView.fromProduct(graphHandle.product()), subgraphHits


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-n", "--maxEvents", type=int, default=-1)
    parser.add_argument("--parent", choices=("tau", "top"), default="tau")
    parser.add_argument("--cone", type=float, default=CONE)
    parser.add_argument("input", help="an EDM file that holds the logical graph and its hit index")
    args = parser.parse_args()
    for number, graph, subgraphHits in events(args.input, args.maxEvents):
        print("== event %s" % number)
        run(graph, subgraphHits, args.parent, args.cone)


if __name__ == "__main__":
    main()
