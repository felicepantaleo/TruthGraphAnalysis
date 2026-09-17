# Vbf

**Preset:** `vbf`. **Sample:** VBFHZZ4Nu vbf.

## The question

What are the dijet mass and the rapidity gap of the two quarks that recoil against the Higgs?

## How the graph answers it

From `signal`, `partonJets`. See `cpp/Vbf.cc` and `py/vbf.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Vbf --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Vbf/py/vbf.py TruthGraphAnalysis/Common/fixtures/vbf.json
```

## Expected output on the fixture

`Common/fixtures/vbf.json`, one event. Both languages print exactly `test/expected.txt`:

```
vbf: 1 Higgs, tagging partons 2 1: m(jj) 372.52 GeV, |delta eta| 4.10
```
