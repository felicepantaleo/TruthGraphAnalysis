# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""Run the graph port of the MTD track validation next to MtdTracksValidation.

Apply it to the reconstruction step of a Run4 workflow. Its input carries the truth graph
and the tracker hit index, which the DIGI step builds:

    cmsDriver.py step3 ... --customise TruthGraphAnalysis/Mtd/customiseMtdGraphValidation.customise

The plots go to MTD/TracksGraph, beside the MTD/Tracks folder of MtdTracksValidation.
"""

import FWCore.ParameterSet.Config as cms


def customise(process):
    from Validation.Configuration.truthPrevalidation_cff import truthLogicalGraphHitIndexProducer
    from SimGeneral.TruthGraphAssociatorProducers.truthGraphAssociators_cff import (
        allTrackToTruthBranchAssociators,
        truthBranchTargets,
    )
    from TruthGraphAnalysis.Mtd.mtdTracksGraphValid_cfi import mtdTracksGraphValid

    # The MTD channel of the hit index: one hit per (sensor module, cell, category) with
    # its time, from the MTD sim clusters. The default DIGI index leaves MTD out.
    process.truthMtdHitIndex = truthLogicalGraphHitIndexProducer.clone(
        src="truthLogicalGraphProducer",
        rawSrc="mix",
        recHitMap="",
        subdetectors=cms.vstring("MTD"),
        trackerDigiSimLinks=[],
        simHitCollections=[],
    )

    # The candidates of the track map. A particle that is not a candidate cannot be
    # matched, and its track then goes to an ancestor, so the candidate floor must sit
    # below the 0.7 GeV of the MTD selection. The shared selector starts at 1 GeV.
    process.mtdTruthBranchTargets = truthBranchTargets.clone()
    process.mtdTruthBranchTargets.branchSelector.ptMin = 0.

    process.mtdTrackToTruth = allTrackToTruthBranchAssociators.clone(
        recoCollections=["generalTracks"],
        targetsSrc=("mtdTruthBranchTargets", "selectedRoots"),
        assignableTargetsSrc=("mtdTruthBranchTargets", "assignableRoots"),
        workingPointNames=["Fixed"],
        adaptiveReverseWeight=[0.],
        adaptiveMaxReverseScore=[0.],
    )

    process.mtdTracksGraphValid = mtdTracksGraphValid.clone()

    process.mtdTracksGraphValidPath = cms.Path(
        process.truthMtdHitIndex
        + process.mtdTruthBranchTargets
        + process.mtdTrackToTruth
        + process.mtdTracksGraphValid
    )
    if process.schedule is not None:
        process.schedule.append(process.mtdTracksGraphValidPath)
    return process
