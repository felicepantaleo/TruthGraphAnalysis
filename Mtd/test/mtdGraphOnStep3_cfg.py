# Runs the graph port of the MTD track validation on a step3.root that holds the truth
# graph and the MTD sim clusters, with no reconstruction. The output is DQMIO.
# cmsRun mtdGraphOnStep3_cfg.py -i step3.root -o mtdGraph_inDQM.root
#
# With --legacy the job also runs MtdTracksValidation on the same events, so one output
# holds both folders. Harvest it with harvestMtd_cfg.py, then compareMtdPort.py compares
# the two folders.
# cmsRun mtdGraphOnStep3_cfg.py -i step3.root -n 20 --legacy -o mtdBoth_inDQM.root
import argparse

import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Phase2C26I13M9_cff import Phase2C26I13M9

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", nargs="+", default=["step3.root"])
parser.add_argument("-n", "--maxEvents", type=int, default=-1)
parser.add_argument("-o", "--output", default="mtdGraph_inDQM.root")
parser.add_argument("-g", "--geometry", default="ExtendedRun4D122")
parser.add_argument("--legacy", action="store_true",
                    help="also run MtdTracksValidation on the same events, into MTD/Tracks")
args = parser.parse_args()

process = cms.Process("MTDGRAPH", Phase2C26I13M9)
process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.load(f"Configuration.Geometry.Geometry{args.geometry}Reco_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag

process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic_T35", "")
process.load("DQMServices.Core.DQMStoreNonLegacy_cff")

process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring(
    [f if ":" in f else "file:" + f for f in args.input]))
process.maxEvents.input = args.maxEvents

from TruthGraphAnalysis.Mtd.customiseMtdGraphValidation import customise

process = customise(process)
process.DQMoutput = cms.OutputModule("DQMRootOutputModule", fileName=cms.untracked.string(args.output))
process.DQMoutputPath = cms.EndPath(process.DQMoutput)
process.schedule = cms.Schedule(process.mtdTracksGraphValidPath, process.DQMoutputPath)

if args.legacy:
    # MtdTracksValidation reads the TrackingParticles and the MTD association maps of the
    # file. It needs the MTD layers and the transient track builder from the EventSetup.
    process.load("RecoMTD.DetLayers.mtdDetLayerGeometry_cfi")
    process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
    process.load("Validation.MtdValidation.mtdTracksValid_cfi")
    # The breakdown of the wrong BTL clusters by origin is an optional plot of the module.
    process.mtdTracksValid.optionalPlots = True
    process.mtdTracksValidPath = cms.Path(process.mtdTracksValid)
    process.schedule.insert(0, process.mtdTracksValidPath)
