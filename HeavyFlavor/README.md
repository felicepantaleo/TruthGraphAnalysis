# HeavyFlavor

**Preset:** `heavyflavor`. **Sample:** TTbar heavyflavor (b hadrons of a ttbar event).

## The question

How far does each b hadron fly before it decays, and how many charm hadrons does it leave below it?

## How the graph answers it

From `branchesAtLevel(BHadrons)`, `cHadrons`, vertex positions. See `cpp/HeavyFlavor.cc` and `py/heavyflavor.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example HeavyFlavor --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/HeavyFlavor/py/heavyflavor.py TruthGraphAnalysis/Common/fixtures/top.json
```

## Expected output on the fixture

`Common/fixtures/top.json`, one event. Both languages print exactly `test/expected.txt`:

```
heavyflavor: 521 pt 54.31 GeV, flight 0.011 cm, 1 charm hadron below
heavyflavor: -511 pt 6.96 GeV, flight 0.096 cm, 2 charm hadrons below
```
