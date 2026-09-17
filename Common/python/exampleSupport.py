# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""What every python example needs beyond graphTools: kinematics from a p4 tuple, a decay
mode, the production siblings, and the first child of a species."""

import math

LEPTONS = (11, 13, 15)
NEUTRINOS = (12, 14, 16)


def mass(p4):
    return math.sqrt(max(p4[3] ** 2 - p4[0] ** 2 - p4[1] ** 2 - p4[2] ** 2, 0.0))


def pt(p4):
    return math.hypot(p4[0], p4[1])


def eta(p4):
    p = math.sqrt(p4[0] ** 2 + p4[1] ** 2 + p4[2] ** 2)
    return 0.5 * math.log((p + p4[2]) / (p - p4[2])) if p > abs(p4[2]) else float("inf")


def add(*p4s):
    return tuple(sum(c) for c in zip(*p4s))


def firstChildWithPdgId(graph, particle, pdgIds):
    """The first child of the last copy whose species is in the list."""
    for child in graph.children(graph.lastCopy(particle)):
        if abs(graph.pdgId(child)) in pdgIds:
            return child
    return None


def decayMode(graph, boson):
    """leptonic, hadronic or none, from the children of the last copy of a W or a Z. A tau
    counts as a lepton here, so W -> tau nu is leptonic whatever the tau does next."""
    pdgIds = [abs(graph.pdgId(c)) for c in graph.children(graph.lastCopy(boson))]
    if any(p in LEPTONS for p in pdgIds):
        return "leptonic"
    if any(p <= 6 for p in pdgIds):
        return "hadronic"
    return "none"


def productionSiblings(graph, particle):
    """The other particles produced where this one was."""
    return [s for v in graph.productionVertices(particle) for s in graph.outgoingParticles(v) if s != particle]


def graphsFrom(paths, maxEvents=-1):
    """One TruthGraphView per input: JSON dumps, or the events of one EDM file."""
    from PhysicsTools.TruthInfo.graphTools import TruthGraphView, eventGraphs

    if paths and paths[0].endswith(".root"):
        return eventGraphs(paths[0], maxEvents=maxEvents)
    return (TruthGraphView.fromJson(path) for path in paths)
