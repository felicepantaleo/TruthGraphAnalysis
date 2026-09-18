# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""What the python examples need beyond graphTools: kinematics from a p4 tuple, the species
sets the C++ interface defines, a decay mode, and the first child of a species."""

import math

LEPTONS = (11, 13, 15)
NEUTRINOS = (12, 14, 16)
# truth::isParton: quarks and gluons.
PARTONS = (1, 2, 3, 4, 5, 6, 21)
# truth::isWeakBoson: the W and the Z, not the Higgs.
WEAK_BOSONS = (23, 24)


def isParton(pdgId):
    return abs(pdgId) in PARTONS


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
    if any(p in PARTONS for p in pdgIds):
        return "hadronic"
    return "none"


def eventNumber(graph):
    """The event number alone, as the C++ plugins print it."""
    eventId = graph.eventId
    return eventId[-1] if isinstance(eventId, tuple) else eventId


def graphsFrom(paths, maxEvents=-1):
    """One TruthGraphView per input: JSON dumps, or the events of one EDM file."""
    from PhysicsTools.TruthInfo.graphTools import TruthGraphView, eventGraphs

    if paths and paths[0].endswith(".root"):
        return eventGraphs(paths[0], maxEvents=maxEvents)
    return (TruthGraphView.fromJson(path) for path in paths)
