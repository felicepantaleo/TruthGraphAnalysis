# SingleTop

**Preset:** `singletop`. **Sample:** ST_tch_top_14TeV_TuneCP5, produced with the custom fragment in `PhysicsTools/TruthInfo/python`.

## The question

What was produced together with the top: the recoil quark, the associated W, or the b?

## How the graph answers it

From `signal`, the production siblings of the top, each named `associated W`, `b`, `recoil quark`, `top` or `other`. A top partner means the event is ttbar. The top is a parton by its pdgId, so it is named before the parton test. See `cpp/SingleTop.cc` and `py/singletop.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example SingleTop --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/SingleTop/py/singletop.py TruthGraphAnalysis/Common/fixtures/singletop.json
```

## Expected output on the fixture

`Common/fixtures/singletop.json`, one event. Both languages print exactly `test/expected.txt`:

```
singletop: top with recoil quark -1 pt 14.49 GeV
```
