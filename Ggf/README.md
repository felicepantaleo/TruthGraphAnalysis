# Ggf

**Preset:** `ggf`. **Sample:** H125GGgluonfusion ggf.

## The question

What does the detector see of the Higgs: how many reconstructable products, and which fraction of its energy is visible?

## How the graph answers it

From `signal`, `reconstructableFromSignal`. The visible fraction is only defined for a colourless decay such as H to gamma gamma or H to ZZ to 4 leptons. In a coloured decay such as H to bb the products hadronize with partons from outside the Higgs, so they carry that energy too and the fraction would exceed 1; the example prints `n/a, coloured decay` instead. See `cpp/Ggf.cc` and `py/ggf.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Ggf --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Ggf/py/ggf.py TruthGraphAnalysis/Common/fixtures/ggf.json
```

## Expected output on the fixture

`Common/fixtures/ggf.json`, one event. Both languages print exactly `test/expected.txt`:

```
ggf: Higgs E 166.44 GeV -> 2 reconstructable products, visible fraction 1.000
```
