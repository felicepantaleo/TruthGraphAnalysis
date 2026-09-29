# Egamma

**Levels:** none. **Preset:** none needed. **Sample:** a `step3.root` with the truth graph and the hit index, for example the zee sample of TruthGraphTutorial.

## The question

Which particle made each GSF track, and does a hit-based match find the same Z electrons as a delta R match?

## How the graph answers it

The release does not associate GSF tracks, so the analyzer runs `truth::BranchHitAssociator` itself, on the tracker channel, with the candidates that `truthBranchTargets` wrote into the file.
A `reco::GsfTrack` is a `reco::Track`, so `truth::recoHits` takes it as it is.
The particle of a track is the best candidate that is not a parton, a boson or a beam particle, when it owns at least 75% of the track hits.
The analyzer takes `lastCopy()` of that particle. The generator writes an electron again after it radiates, both copies own the same hits, and on a tie the associator puts the earlier copy first.

A Z electron is a generator-stable electron with a Z ancestor, pT > 5 GeV and abs(eta) < 3. It is found by hits when a GSF track belongs to it, and by delta R when a GSF track is within 0.05 of it.

## Run it

```bash
cmsRun TruthGraphAnalysis/Egamma/test/gsfTrackTruth_cfg.py -i step3.root -o gsfTrackTruth.root
```

## Expected output

TruthGraphTutorial zee, 1000 events:

```
GSF tracks 1655: 1652 with a particle owning 75% of the hits, 1496 of them an electron, 1484 an electron from the Z
Z electrons 1552: found by hits 1391, by delta R 1391, by both 1391
```
