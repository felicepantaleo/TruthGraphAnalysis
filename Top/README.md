# Top

**Preset:** `top`. **Sample:** TTbar top.

## The question

For each top: is there a b, how did the W decay, how large is its subgraph; and what class is the event, all hadronic, semileptonic or dilepton? A W to tau nu counts as leptonic.

## How the graph answers it

From `signal`, `forEachChildId`, descendants. See `cpp/Top.cc` and `py/top.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Top --rebuild

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Top/py/top.py TruthGraphAnalysis/Common/fixtures/top.json
```

## Expected output on the fixture

`Common/fixtures/top.json`, one event. Both languages print exactly `test/expected.txt`:

```
top 6: b yes, W hadronic, 795 descendants
top -6: b yes, W hadronic, 532 descendants
top event class: all hadronic
```
