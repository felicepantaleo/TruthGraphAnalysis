# Tau

**Levels:** `tauVisibleHadronic`, `tauVisibleLeptonic`. **Preset:** none needed. **Sample:** TenTau_E_15_500 gun, or any sample with taus.

## The question

How did each tau decay: how many charged hadrons and neutral pions, which decay mode tau reconstruction would call it, and how much of the tau energy is visible?

## How the graph answers it

The taus are the members of the two tau levels, asked for in one call with `truth::particlesAtLevels`. Each member is the last copy of a physical tau, so a tau that radiates a photon is counted once. The levels are graph levels, not the selection's signal, so the example runs on a graph built with no preset.

`truth::tauDecay` says whether the decay is hadronic or leptonic, the rule `TauGenJetProducer` applies. For a hadronic tau, the example follows the generator decay chain down from the tau and counts what leaves it:

- It goes through an intermediate resonance, such as the rho, the a1 or the omega, so a three-prong decay through an a1 counts its three pions.
- It stops at a pi0 and counts it, without following it to its photons.
- It stops at a K0S or a K0L and counts it as neutral, not as a prong.
- It stops at a particle the generator did not decay. A charged pion or kaon is a prong.
- It follows only generator decay vertices. A charged pion that interacts in the tracker makes new particles at a Geant4 vertex, and none of them is a decay product.

The decay mode is named `<prongs>prong<pi0s>pi0`, with `<n>K0` added when a neutral kaon is present, or `electron` and `muon` for a leptonic decay. The `reco mode` is the number `reco::PFTau::hadronicDecayMode` gives the same content: `5 * (prongs - 1) + pi0s` with the pi0 count capped at 4, 15 (`kRareDecayMode`) outside one to three prongs, and -1 (`kNull`) for a leptonic decay, which tau reconstruction does not classify.

The visible energy is the energy of everything the walk reached, the neutrinos excluded, over the tau energy.

See `cpp/Tau.cc` and `py/tau.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with the branch and this repository checked out. The tau levels
# need no preset, so the graph in the file is used as it is.
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Tau

# python, on a JSON dump or an EDM file
python3 TruthGraphAnalysis/Tau/py/tau.py TruthGraphAnalysis/Common/fixtures/tentau.json
```

## Expected output on the fixture

`Common/fixtures/tentau.json`, one event with ten taus. Both languages print exactly `test/expected.txt`:

```
tau -15 E 113.18 GeV: 1prong0pi0, reco mode 0, 1 prongs 0 pi0 0 photons, visible energy 0.40
tau 15 E 113.18 GeV: 1prong0pi0, reco mode 0, 1 prongs 0 pi0 0 photons, visible energy 0.02
tau -15 E 37.37 GeV: 3prong0pi0, reco mode 10, 3 prongs 0 pi0 0 photons, visible energy 0.47
tau 15 E 37.37 GeV: 3prong0pi0, reco mode 10, 3 prongs 0 pi0 0 photons, visible energy 0.91
tau -15 E 26.27 GeV: electron, reco mode -1, 0 prongs 0 pi0 0 photons, visible energy 0.18
tau 15 E 26.27 GeV: 3prong1pi0, reco mode 11, 3 prongs 1 pi0 0 photons, visible energy 0.84
tau -15 E 107.25 GeV: 1prong1pi0, reco mode 1, 1 prongs 1 pi0 0 photons, visible energy 0.69
tau 15 E 107.25 GeV: 1prong1pi0, reco mode 1, 1 prongs 1 pi0 0 photons, visible energy 0.40
tau -15 E 177.58 GeV: 3prong0pi0, reco mode 10, 3 prongs 0 pi0 0 photons, visible energy 0.61
tau 15 E 177.58 GeV: electron, reco mode -1, 0 prongs 0 pi0 0 photons, visible energy 0.19
mode 1prong0pi0: 2
mode 1prong1pi0: 2
mode 3prong0pi0: 3
mode 3prong1pi0: 1
mode electron: 2
```

The second tau keeps only 2% of its energy visible: it decays to a neutrino of 110.95 GeV and a pi- of 2.23 GeV, which is allowed for tau to pi nu.

## What the fixture does not exercise

The event holds no muonic decay, no neutral kaon, no radiated photon and no mode with more than one pi0. The code handles them, but the test does not prove it on this fixture.
