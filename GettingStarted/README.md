# GettingStarted

Four short python scripts for a first look at the truth graph. Each one reads a `step3.root` or a URL of the TruthGraphTutorial samples.

| script | question |
|---|---|
| `py/first_look.py` | What does one event hold: particles, vertices, interactions, and the decay of each top? |
| `py/tau_products.py` | What are the visible decay products of each tau, without the particles that Geant4 adds? |
| `py/z_mass.py` | What is the mass of the two leptons of the Z? |
| `py/pileup.py` | How many interactions does a pileup event hold, and how many particles come from the signal? |

## Run them

```bash
T=https://felice.web.cern.ch/orbit/TruthGraphTutorial
python3 TruthGraphAnalysis/GettingStarted/py/first_look.py $T/ttbar/step3.root -n 2
python3 TruthGraphAnalysis/GettingStarted/py/tau_products.py $T/tentau/step3.root
python3 TruthGraphAnalysis/GettingStarted/py/z_mass.py $T/zmumu/step3.root -o z_mass.png
python3 TruthGraphAnalysis/GettingStarted/py/pileup.py $T/ttbar_pu200/step3.root
```

The expected output of each script is on the page "Getting started: recipes and FAQ" of https://cms-truth.docs.cern.ch/.
