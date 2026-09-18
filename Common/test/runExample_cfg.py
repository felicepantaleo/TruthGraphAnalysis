# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

# Runs one C++ example plugin over a file that holds the logical graph:
#
#   cmsRun runExample_cfg.py step3.root --example Top --rebuild
#   cmsRun runExample_cfg.py step3.root --example Gun --rebuild --fragment TenTau_E_15_500
#
# A production file holds the graph with no selection preset, so the signal level is
# empty there. --rebuild builds the graph again from the GEN and SIM record in the file,
# with the preset's selection. A gun takes its species from the fragment name.

import FWCore.ParameterSet.Config as cms
from argparse import ArgumentParser

PRESET_OF = {
    "Gun": "gun",
    "Resonance": "resonance",
    "Vbf": "vbf",
    "Ggf": "ggf",
    "Vh": "vh",
    "Top": "top",
    "SingleTop": "singletop",
    "Diboson": "diboson",
    "HeavyFlavor": "heavyflavor",
    "Full": "full",
}

parser = ArgumentParser()
parser.add_argument("inputFile", nargs="?", default="step3.root")
parser.add_argument("--example", required=True, choices=sorted(PRESET_OF))
parser.add_argument("-n", "--maxEvents", type=int, default=3)
parser.add_argument("--src", default="truthLogicalGraphProducer", help="label of the logical graph")
parser.add_argument("--rebuild", action="store_true",
                    help="build the graph from the file's GEN and SIM record with the preset's selection")
parser.add_argument("--fragment", default=None,
                    help="with --rebuild, resolve the selection from this generator fragment name")
args = parser.parse_args()
if "/" not in args.inputFile and ":" not in args.inputFile:
    args.inputFile = "file:" + args.inputFile

process = cms.Process("EXAMPLE")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(args.maxEvents))
process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring(args.inputFile))
process.options = cms.untracked.PSet(wantSummary=cms.untracked.bool(False))

process.example = cms.EDAnalyzer("TruthGraph%sExample" % args.example, src=cms.InputTag(args.src))
process.p = cms.Path(process.example)

if args.rebuild:
    from PhysicsTools.TruthInfo.customiseTruthPreset import applyTruthPreset
    from Validation.Configuration.truthPrevalidation_cff import truthLogicalGraphProducer as _logical
    process.truthGraphProducer = cms.EDProducer(
        "TruthGraphProducer",
        genEventHepMC3=cms.InputTag("generatorSmeared"),
        genEventHepMC=cms.InputTag("generatorSmeared"),
        simTracks=cms.InputTag("g4SimHits"),
        simVertices=cms.InputTag("g4SimHits"),
        addGenToSimEdges=cms.bool(True),
    )
    process.truthLogicalGraphProducer = _logical.clone(src=cms.InputTag("truthGraphProducer"))
    # One call sets the preset on every module that has to agree on it, and prints which
    # preset it resolved. Name the fragment, or name the preset.
    if args.fragment is not None:
        applyTruthPreset(process, fragment=args.fragment)
    else:
        applyTruthPreset(process, preset=PRESET_OF[args.example])
    process.example.src = cms.InputTag("truthLogicalGraphProducer")
    process.p.insert(0, process.truthGraphProducer + process.truthLogicalGraphProducer)

process.MessageLogger.cerr.threshold = "WARNING"
