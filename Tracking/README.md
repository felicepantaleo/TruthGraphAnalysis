# Tracking

**Levels:** none. **Preset:** none needed. **Sample:** any `step3.root` with the truth association maps, for example the ttbar sample of TruthGraphTutorial.

## The question

What are the track efficiency, the fake rate and the duplicate rate of `generalTracks`, measured with the truth graph and the definitions of `MultiTrackValidator`?

## How the graph answers it

The analyzer reads the maps that `customiseTruthGraphAssociators` writes into `step3.root`:

- `allTrackToTruthBranchAssociators:generalTracksTruthToReco`: one row per particle, entries (track, shared hits, score).
- `allTrackToTruthBranchAssociators:generalTracksRecoToTruthFixed`: one row per track, entries (particle, shared hits, score). Row [0] is the best detector particle.

The selected particles are the ones `MultiTrackValidator` selects for its efficiency against eta: simulated, charged, signal and in time, pT > 0.9 GeV, |eta| < 4.5, production vertex with r < 2.5 cm and |z| < 30 cm.
A track comes from a particle when the particle owns at least 75% of the track hits, and all three hits of a three-hit track.
A particle is found when one track comes from it, and duplicated when two or more tracks come from it.
A track is fake when its best particle does not own 75% of its hits.

The maps in the file offer as candidates only the stable particles with pT > 1 GeV and |eta| < 4.
Tracks of softer particles have no candidate, and the analyzer counts them as fake.
`--candidatePtMin` runs the association again in the job with a wider candidate set.

## Run it

```bash
cmsRun TruthGraphAnalysis/Tracking/test/trackingEfficiency_cfg.py -i step3.root -o trackingEfficiency.root
# with every particle above 0.1 GeV and |eta| < 4.5 as a candidate
cmsRun TruthGraphAnalysis/Tracking/test/trackingEfficiency_cfg.py -i step3.root --candidatePtMin 0.1
```

The output file holds the numerators and the denominators against eta and pT. The job prints the totals.

## Expected output

TruthGraphTutorial ttbar, 1000 events, maps from the file:

```
selected particles 71985, efficiency 0.90623, duplicate rate 0.00266722; tracks 149286, fake rate 0.00111866
```

The `MultiTrackValidator` of the same job selects 71974 TrackingParticles and 149286 tracks. Its efficiency is 0.889 and its fake rate 0.0048.
