# ParticleFlow

**Levels:** none. **Preset:** none needed. **Sample:** a `step3.root` with the truth association maps, for example the ttbar sample of TruthGraphTutorial.

## The question

Does the type of each PF candidate agree with the particle that made it, and what fraction of the particle energy does the candidate carry?

## How the graph answers it

A charged candidate takes the particle of its track: row [0] of `allTrackToTruthBranchAssociators:generalTracksRecoToTruthFixed`.
A neutral candidate takes the particle of its most energetic ECAL or HCAL cluster: row [0] of the `particleFlowClusterECALRecoToTruthFixed` or `particleFlowClusterHCALRecoToTruthFixed` map.
A match counts when its score is at most 0.25, so the particle owns at least 75% of the object.

The truth classes are e, mu, gamma, charged hadron, neutral hadron and `merged`.
`merged` is a particle that the generator decayed, for example a pi0, an eta or a B meson. It is the best match when the object merges several of its decay products.
The endcap neutral candidates come from HGCAL through TICL and have no ECAL or HCAL cluster, so the analyzer counts them and skips them.

## Run it

```bash
cmsRun TruthGraphAnalysis/ParticleFlow/test/pfCandidateTruth_cfg.py -i step3.root --minEnergy 5
```

## Expected output

TruthGraphTutorial ttbar, 1000 events, candidates above 5 GeV:

```
PF type vs truth class (rows: PF type)
               e        mu     gamma       h+-        h0    merged  no match
     h      1216       100        34     47583        45        91       141
     e       555         3         4       716         0         6        20
    mu         3       571         0       121         0         1         0
 gamma        90         0       572       165       295      2591       204
    h0         3         0         2       294       361       365       222
neutral candidates without an ECAL or HCAL cluster (HGCAL or HF): 105987
mean E response: charged 0.977653, neutral 0.527128
```
