# Full

**Preset:** `full`. **Sample:** any sample; the fixture is a ttbar event.

## The question

What does the whole event hold, interaction by interaction, and how large is the reconstructable final state of the signal and of the pile-up?

## How the graph answers it

From `reconstructableFinalState`, `isSignal`, `hasMomentum`. See `cpp/Full.cc` and `py/full.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Full --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Full/py/full.py TruthGraphAnalysis/Common/fixtures/top.json
```

## Expected output on the fixture

`Common/fixtures/top.json`, one event. Both languages print exactly `test/expected.txt`:

```
full: interaction bx 0 index 0: 2380 particles
full: signal reconstructable final state 349 objects, 11903.8 GeV, 0 without momentum
full: pileup reconstructable final state 0 objects, 0.0 GeV, 0 without momentum
```
