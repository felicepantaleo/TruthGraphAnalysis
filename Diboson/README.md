# Diboson

**Preset:** `diboson`. **Sample:** WWTo2L2Nu_14TeV_TuneCP5, produced with the custom fragment in `PhysicsTools/TruthInfo/python`.

## The question

What is the mass of the boson pair, and how did each boson decay?

## How the graph answers it

From `signal`. See `cpp/Diboson.cc` and `py/diboson.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Diboson --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Diboson/py/diboson.py TruthGraphAnalysis/Common/fixtures/diboson.json
```

## Expected output on the fixture

`Common/fixtures/diboson.json`, one event. Both languages print exactly `test/expected.txt`:

```
diboson: m(VV) 160.09 GeV
  -24 pt 24.39 GeV, decay leptonic
  24 pt 46.13 GeV, decay leptonic
```
