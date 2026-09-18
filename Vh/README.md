# Vh

**Preset:** `vh`. **Sample:** ZHToBB_ZToLL_14TeV_TuneCP5, produced with the custom fragment in `PhysicsTools/TruthInfo/python`.

## The question

Which vector boson was produced together with the Higgs, and how did it decay?

## How the graph answers it

From `signal`, the production siblings of the Higgs. See `cpp/Vh.cc` and `py/vh.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Vh --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Vh/py/vh.py TruthGraphAnalysis/Common/fixtures/vh.json
```

## Expected output on the fixture

`Common/fixtures/vh.json`, one event. Both languages print exactly `test/expected.txt`:

```
vh: Higgs with 23 pt 10.88 GeV, boson decay leptonic
```
