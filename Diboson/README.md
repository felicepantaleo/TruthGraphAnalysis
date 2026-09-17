# Diboson

**Preset:** `diboson`. **Sample:** WWTo2L2Nu diboson (the fixture is a ttbar event seeded on its two W).

## The question

What is the mass of the boson pair, and how did each boson decay?

## How the graph answers it

From `signal`. See `cpp/Diboson.cc` and `py/diboson.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Diboson --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Diboson/py/diboson.py TruthGraphAnalysis/Common/fixtures/top_diboson_seeds.json
```

## Expected output on the fixture

`Common/fixtures/top_diboson_seeds.json`, one event. Both languages print exactly `test/expected.txt`:

```
diboson: m(VV) 205.66 GeV
  24 pt 57.06 GeV, decay hadronic
  -24 pt 66.60 GeV, decay hadronic
```
