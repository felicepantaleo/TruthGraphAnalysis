# Mtd: port an existing validation to the truth graph

The question: `MtdTracksValidation` decides, for each track, whether the MTD cluster on the
track was made by the particle that made the track. It reads the answer from
TrackingParticles and from the MTD sim clusters. Can the truth graph give the same answer,
with the same conditions, in less code?

This example is the port. It is a DQM analyser, `MtdTracksGraphValidation`, that runs in
the same job as `MtdTracksValidation`, on the same tracks, and fills the monitor elements
of the legacy module under the same names in `MTD/TracksGraph`. The step-by-step guide is
the tutorial page "Port an existing associator" on the documentation site.

## What changes and what does not

The reco side is identical: the same tracks, the same track selection, the same MTD
clusters on each track, the same time `t0safe`. Only the truth questions change source.

| question | `MtdTracksValidation` | graph port |
|---|---|---|
| Which particle made the track? | `trackingParticleRecoTrackAsssociation`, first in-time TP | track map `generalTracksRecoToTruthFixed`, best in-time particle with score at most 0.25 |
| In time? | `tp.eventId().bunchCrossing() == 0` | `ParticleData::bunchCrossing() == 0` |
| Charged, eta, pt | `tp.charge()`, `tp.eta()`, `tp.pt()` | `Particle::threeCharge()`, `ParticleData::momentum` |
| Produced inside the MTD volume | `tp.parentVertex()->position()` | the production vertex, in cm and ns |
| Generator final state | `tp.status() == 1` | `ParticleData::status == 1` |
| What did it leave in the MTD, directly or through a secondary, and when? | `mtdSimLayerClusterToTPAssociation`, `hitProdType()`, `simLCTime()` | `directHits(HitChannel::MTD, particle)` and `directHitTimes(...)`: module, cell, category, energy, time |
| Is the track cluster its own? | `mtdRecoClusterToSimLayerClusterAssociation`, then the TP clusters | a shared cell, the time cut and, in BTL, the energy cut, as the legacy associator |

## Layout

```
Mtd/
  cpp/MtdTruth.h                   selection, production point, MTD footprint: free functions
  cpp/MtdTruth.cc
  cpp/MtdTracksGraphValidation.cc  the DQM analyser
  python/customiseMtdGraphValidation.py   schedules the port in a RECO job
  test/Mtd_t.cc                    unit tests of the free functions
  test/compareMtdPort.py           prints the legacy and the graph counts side by side
```

## Running it

Apply the customise to the reconstruction step of a Run4 workflow. The DIGI step already
wrote the graph and the tracker hit index.

```bash
cmsDriver.py step3 ... --customise TruthGraphAnalysis/Mtd/customiseMtdGraphValidation.customise
# harvest as usual, then
python3 TruthGraphAnalysis/Mtd/test/compareMtdPort.py DQM_V0001_R000000001__Global__CMSSW_X_Y_Z__RECO.root
```

The customise does three things the default job does not:

1. It builds the MTD channel of the hit index, `truthMtdHitIndex`, which the default DIGI
   index leaves out. It reads the MTD sim clusters and the MTD topology.
2. It builds the track map with a candidate floor of 0 GeV. The shared selector starts at
   1 GeV. A particle that is not a candidate cannot be matched, and its track then goes to
   an ancestor whose subgraph holds the same hits. The floor must sit below the 0.7 GeV of
   the MTD selection.
3. It schedules `MtdTracksGraphValidation` after them.

## Result

Both modules in one job, Run4 D127 ttbar, tracks above 0.7 GeV. It needs the follow-up
branch `truth-association-tie-descendant` of PR 51829, which carries the cell-keyed MTD
channel with times and categories, and the tie rule of the track map.

| category | no pileup: legacy | graph | 5 pileup: legacy | graph |
|---|---:|---:|---:|---:|
| BTL: particle with direct hits | 2792 | 2782 | 1289 | 1286 |
| BTL: the first direct cluster on the track | 2512 | 2505 | 1106 | 1104 |
| BTL: a later direct cluster on the track | 68 | 66 | 50 | 49 |
| BTL: particle with other hits only | 585 | 595 | 296 | 297 |
| ETL: particle with MTD hits | 1853 | 1841 | 986 | 978 |
| ETL: correct cluster on the track | 1360 | 1395 | 725 | 742 |
| BTL time residual, mean and RMS | -9.5, 32.8 ps | -9.4, 32.6 ps | -9.5, 30.4 ps | -9.5, 30.4 ps |

The documentation page gives the full table and the measured cause of every difference.

## Limits

- A deposit is one (module, category) group. The legacy accumulator splits a group whose
  cells are not adjacent; 6.3% of the BTL groups on ttbar.
- The wrong-cluster breakdown uses the order "a later direct deposit, an other deposit, not
  its deposit"; the legacy module uses the last sim cluster it looked at.
- The module needs an MTD channel with cells and times. It logs an error once when the
  channel has neither.
