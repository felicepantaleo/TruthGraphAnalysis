# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

# Harvests the DQMIO output of mtdGraphOnStep3_cfg.py into one DQM file, which
# compareMtdPort.py reads. The job runs no harvesting module: the comparison reads the
# entries of the monitor elements the two analysers fill.
# cmsRun harvestMtd_cfg.py -i mtdBoth_inDQM.root
# writes DQM_V0001_R000000001__TruthGraphAnalysis__Mtd__Port.root
import argparse

import FWCore.ParameterSet.Config as cms

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", nargs="+", default=["mtdBoth_inDQM.root"])
args = parser.parse_args()

process = cms.Process("HARVESTING")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("DQMServices.Core.DQMStoreNonLegacy_cff")
process.load("DQMServices.Components.DQMFileSaver_cfi")

process.source = cms.Source("DQMRootSource", fileNames=cms.untracked.vstring(
    [f if ":" in f else "file:" + f for f in args.input]))
process.maxEvents.input = -1

process.dqmSaver.convention = "Offline"
process.dqmSaver.workflow = "/TruthGraphAnalysis/Mtd/Port"
process.dqmSaver.saveByRun = -1
process.dqmSaver.saveAtJobEnd = True
process.dqmSaver.forceRunNumber = 1

process.dqmSaverPath = cms.EndPath(process.dqmSaver)
