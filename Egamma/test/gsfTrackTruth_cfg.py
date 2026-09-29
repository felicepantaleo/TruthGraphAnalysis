# Runs GsfTrackTruthAnalyzer on a step3.root that holds the truth graph and the truth association maps.
# cmsRun gsfTrackTruth_cfg.py -i step3.root -n 100 -o gsfTrackTruth.root
import argparse
import FWCore.ParameterSet.Config as cms

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", nargs="+", default=["step3.root"])
parser.add_argument("-n", "--maxEvents", type=int, default=-1)
parser.add_argument("-o", "--output", default="gsfTrackTruth.root")
args = parser.parse_args()

process = cms.Process("ANALYSIS")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring(
    [f if ":" in f else "file:" + f for f in args.input]))
process.maxEvents.input = args.maxEvents
process.TFileService = cms.Service("TFileService", fileName=cms.string(args.output))
process.analyzer = cms.EDAnalyzer("GsfTrackTruthAnalyzer")
process.p = cms.Path(process.analyzer)
