# Gun

**Preset:** `gun`. **Sample:** TenTau_E_15_500 gun.

## The question

Per gun particle: how many reconstructable products does it have, and how many of its descendants reach the calorimeter?

## How the graph answers it

From `signal`, `reconstructableFromSignal`, `caloBoundary`. See `cpp/Gun.cc` and `py/gun.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Gun --rebuild --fragment TenTau_E_15_500

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Gun/py/gun.py TruthGraphAnalysis/Common/fixtures/tentau.json
```

## Expected output on the fixture

`Common/fixtures/tentau.json`, one event. Both languages print exactly `test/expected.txt`:

```
gun -15 E 113.18 GeV: 1 reconstructable products, 1 descendants reach the calorimeter
gun 15 E 113.18 GeV: 1 reconstructable products, 1 descendants reach the calorimeter
gun -15 E 37.37 GeV: 3 reconstructable products, 12 descendants reach the calorimeter
gun 15 E 37.37 GeV: 3 reconstructable products, 3 descendants reach the calorimeter
gun -15 E 26.27 GeV: 1 reconstructable products, 1 descendants reach the calorimeter
gun 15 E 26.27 GeV: 4 reconstructable products, 23 descendants reach the calorimeter
gun -15 E 107.25 GeV: 2 reconstructable products, 3 descendants reach the calorimeter
gun 15 E 107.25 GeV: 2 reconstructable products, 105 descendants reach the calorimeter
gun -15 E 177.58 GeV: 3 reconstructable products, 61 descendants reach the calorimeter
gun 15 E 177.58 GeV: 1 reconstructable products, 1 descendants reach the calorimeter
```
