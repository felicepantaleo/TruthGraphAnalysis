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
| Charged, eta, pt | `tp.charge()`, `tp.eta()`, `tp.pt()` | `Particle::charge()`, `ParticleData::momentum` |
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
  test/mtdGraphOnStep3_cfg.py      runs the port on a step3.root; with --legacy, also MtdTracksValidation
  test/harvestMtd_cfg.py           harvests the DQMIO output into one DQM file
  test/compareMtdPort.py           prints the legacy and the graph counts side by side
  test/testMtdPort.sh              the regression test: the three steps above on 20 events
  test/expected_compare.txt        the table the regression test must print
```

## Running it

Apply the customise to the reconstruction step of a Run4 workflow. The DIGI step already
wrote the graph and the tracker hit index.

```bash
cmsDriver.py step3 ... --customise TruthGraphAnalysis/Mtd/customiseMtdGraphValidation.customise
# harvest as usual, then
python3 TruthGraphAnalysis/Mtd/test/compareMtdPort.py DQM_V0001_R000000001__Global__CMSSW_X_Y_Z__RECO.root
```

To run the port on a finished `step3.root`, with no reconstruction, use `--legacy`. The
job then runs `MtdTracksValidation` on the same events, from the TrackingParticles, the MTD
sim clusters and the association maps that the file holds:

```bash
cmsRun TruthGraphAnalysis/Mtd/test/mtdGraphOnStep3_cfg.py -i step3.root --legacy -o mtdBoth_inDQM.root
cmsRun TruthGraphAnalysis/Mtd/test/harvestMtd_cfg.py -i mtdBoth_inDQM.root
python3 TruthGraphAnalysis/Mtd/test/compareMtdPort.py DQM_V0001_R000000001__TruthGraphAnalysis__Mtd__Port.root
```

`--legacy` sets `optionalPlots` of the legacy module, which fills the breakdown of the
wrong BTL clusters. Without `--legacy` the job runs the port only. Its output can go to a
standard harvesting job together with the `step3_inDQM.root` of the same events, which
holds the legacy folder for all events of the file:

```bash
cmsRun TruthGraphAnalysis/Mtd/test/mtdGraphOnStep3_cfg.py -i step3.root -o mtdGraph_inDQM.root
cmsDriver.py step4 -s HARVESTING:@phase2Validation -n -1 --conditions auto:phase2_realistic_T35 \
    --geometry ExtendedRun4D122 --era Phase2C26I13M9 --mc --scenario pp --filetype DQM \
    --filein file:mtdGraph_inDQM.root,file:step3_inDQM.root
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

Both modules in one job, Run4 D127 ttbar, tracks above 0.7 GeV. It needs the cell-keyed
MTD channel with times and categories, and the tie rule of the track map.
`CMSSW_20_1_0_pre5` holds both.

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

## Test

`scram b runtests` runs two tests. `Mtd_t` tests the free functions. `MtdPort_t`
(`test/testMtdPort.sh`) compares the port with the legacy module:

1. One job runs `MtdTracksValidation` and `MtdTracksGraphValidation` on the first 20
   events of the ttbar tutorial sample (Run4 D122, no pileup). It reads
   `https://felice.web.cern.ch/orbit/TruthGraphTutorial/ttbar/step3.root` directly,
   and reads 55 MB of the 8.2 GB.
2. `harvestMtd_cfg.py` harvests the output.
3. The output of `compareMtdPort.py` must be equal to `test/expected_compare.txt`,
   character by character: every count of the two folders, and the mean and the RMS of
   the time residual in ps with one decimal.

The test takes 25 to 40 s at CERN, and 1.5 to 2 min with the site configuration of the
CI job. It fails when the input cannot be read, when a job fails, when a job takes more
than 900 s, and when a number changes. It needs the network: the input over https, and
the conditions of `auto:phase2_realistic_T35` from Frontier.
`MTD_PORT_TEST_INPUT` names another copy of the file, for example `file:/data/step3.root`.

The numbers in `expected_compare.txt` have two independent checks on the same 20 events.
The legacy column is equal to the output of a job that runs only `MtdTracksValidation`
with `optionalPlots`.
The graph column is equal to the output of `mtdGraphOnStep3_cfg.py` without `--legacy`.
After an intended change of the port, make the file again with the three commands of
"Running it" and `-n 20`, and explain each number that moves.

The CI job sets `SITECONFIG_PATH` to the site configuration `T3_US_OSG` of cvmfs. The
default configuration of cvmfs is the one of CERN. It takes the Frontier proxies from a
host that resolves only inside CERN, and the connection to the conditions fails when the
host does not resolve. `T3_US_OSG` uses the public proxy discovery and the public
Frontier servers. With this setting the test passes inside the container of the CI job
(`cmssw/el8:x86_64`) at CERN. No run on a GitHub runner exists yet.

## Limits

- A deposit is one (module, category) group. The legacy accumulator splits a group whose
  cells are not adjacent; 6.3% of the BTL groups on ttbar.
- The wrong-cluster breakdown uses the order "a later direct deposit, an other deposit, not
  its deposit"; the legacy module uses the last sim cluster it looked at.
- The module needs an MTD channel with cells and times. It logs an error once when the
  channel has neither.
