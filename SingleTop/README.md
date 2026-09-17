# SingleTop

**Preset:** `singletop`. **Sample:** ST_tch_top singletop (the fixture is a ttbar event, so the partner is the other top).

## The question

What was produced together with the top: the recoil quark, the associated W, or the b?

## How the graph answers it

From `signal`, the production siblings of the top. See `cpp/SingleTop.cc` and `py/singletop.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example SingleTop --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/SingleTop/py/singletop.py TruthGraphAnalysis/Common/fixtures/top.json
```

## Expected output on the fixture

`Common/fixtures/top.json`, one event. Both languages print exactly `test/expected.txt`:

```
singletop: top with partner -6 pt 55.76 GeV
singletop: top with partner 6 pt 49.58 GeV
```
