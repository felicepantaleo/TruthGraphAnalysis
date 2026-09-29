# Track efficiency, fake and duplicate rate from the truth graph, on a step3.root that
# holds the truth association maps.
# cmsRun trackingEfficiency_cfg.py -i step3.root -n 100 -o trackingEfficiency.root
# --candidatePtMin X reruns the track association in this job, with every particle above
# X GeV and |eta| < 4.5 as a candidate, instead of reading the maps in the file.
import argparse
import FWCore.ParameterSet.Config as cms

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", nargs="+", default=["step3.root"])
parser.add_argument("-n", "--maxEvents", type=int, default=-1)
parser.add_argument("-o", "--output", default="trackingEfficiency.root")
parser.add_argument("--candidatePtMin", type=float, default=None)
args = parser.parse_args()

process = cms.Process("TRACKEFF")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring(
    [f if ":" in f else "file:" + f for f in args.input]))
process.maxEvents.input = args.maxEvents
process.TFileService = cms.Service("TFileService", fileName=cms.string(args.output))
process.trackingEfficiency = cms.EDAnalyzer("TrackingEfficiencyAnalyzer")
process.p = cms.Path(process.trackingEfficiency)

if args.candidatePtMin is not None:
    # The maps in the file have the candidates of the release: stable particles with
    # pT > 1 GeV and |eta| < 4. The analyzer reads the maps of this process, which is
    # the latest one.
    from SimGeneral.TruthGraphAssociatorProducers.truthGraphAssociators_cff import (
        truthBranchTargets, allTrackToTruthBranchAssociators)
    process.truthBranchTargets = truthBranchTargets.clone()
    process.truthBranchTargets.branchSelector.ptMin = args.candidatePtMin
    process.truthBranchTargets.branchSelector.etaMin = -4.5
    process.truthBranchTargets.branchSelector.etaMax = 4.5
    process.allTrackToTruthBranchAssociators = allTrackToTruthBranchAssociators.clone()
    process.p.insert(0, process.truthBranchTargets + process.allTrackToTruthBranchAssociators)
