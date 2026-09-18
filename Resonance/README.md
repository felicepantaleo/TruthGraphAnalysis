# Resonance

**Preset:** `resonance`. **Sample:** ZEE resonance.

## The question

Which two leptons does the Z decay to, and how much of the Z mass does final-state radiation take away from them?

## How the graph answers it

From `signal`, the children of the Z. The decay legs carry the Z four-vector exactly, so their mass is the generator mass by construction. `Particle::lastCopy()` of each lepton is what is left after it radiated photons, which is what a detector measures; the difference is the final-state radiation. See `cpp/Resonance.cc` and `py/resonance.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Resonance --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Resonance/py/resonance.py TruthGraphAnalysis/Common/fixtures/zee.json
```

## Expected output on the fixture

`Common/fixtures/zee.json`, one event. Both languages print exactly `test/expected.txt`:

```
resonance 23 -> 11 -11: generator mass 87.25 GeV, m(ll) 87.25 GeV from the decay, 75.91 GeV after FSR
```
