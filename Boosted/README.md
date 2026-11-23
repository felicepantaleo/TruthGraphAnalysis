# Boosted

**Levels:** `tauVisibleHadronic`, `tauVisibleLeptonic` for taus; the last copy of each top for tops. **Preset:** none needed. **Sample:** any sample with taus or tops, with the logical graph and its hit index. No boosted sample exists yet.

## The question

In a boosted tau or top the decay products are close, and their footprints in the calorimeter overlap. Which cells do the products share, and how much of the energy is in shared cells? Does each product keep its label: from the hits, and from a Delta R match?

## How the graph answers it

The parents are the taus of the two tau levels, or the last copy of each top. A top is a member of no level on a production file, because the signal level needs the preset `top`. The example cannot build the graph again with a preset: the hit index of the file names the particles of the graph of the file.

The products of a parent are the particles that leave its generator decay vertices, the neutrinos excluded. A tau gives its pions, kaons, pi0s or its charged lepton. A top gives a b and a W. The W stays one product: its two quarks hadronize together, so the graph gives the same hadrons to both. On ttbar event 16 the two quarks of a W have the same 1547 cells.

The hits of a product are its calorimeter hits and those of its descendants, from `LogicalGraphHitIndex::appendSubgraphHits` on the channel `Calo`. The direct hits alone are not enough: a pi0, a quark and a W have none. On ttbar event 15 the four children of the two tops have 0 direct hits and 403 to 2235 cells in their subgraphs. In the calorimeter channel a DetId is a cell. A product can have several hits in one cell, so the example adds them.

The hit logic needs no graph. It is `report` in `cpp/Boosted.cc` and in `py/boosted.py`, and it takes a list of products, each with a species, an axis and a hit list:

- A **shared cell** is a cell with energy from two or more products of the parent.
- The **shared energy fraction** is the energy of all products in the shared cells over the energy of all products.
- The **purity** of a product is its energy over the energy of all products in its cells.
- The **hit label** of a product is kept when its purity is above 0.5: the product owns most of the energy in its cells.
- The **Delta R label** of a product is kept when no other product axis is inside a cone of 0.4 around its axis. With two axes in one cone, a Delta R match cannot say which product an object comes from. The cone is the radius of an AK4 jet, and it is a parameter (`cone`).
- **same label** says whether the two methods agree for the product.

The last line of a parent counts the labels that each method keeps.

See `cpp/Boosted.cc` and `py/boosted.py`: the two print the same lines.

## Run it

```bash
# C++, in a CMSSW area with this repository checked out. The file must hold the graph and
# its hit index, as a step3.root of the TruthGraphTutorial samples does.
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Boosted
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Boosted --parent top

# python, on the same file
python3 TruthGraphAnalysis/Boosted/py/boosted.py step3.root -n 3 --parent top
```

## Expected output on the hand-written hit lists

No fixture of a real boosted event exists yet. The tests give `report` four hit lists written by hand, and both languages print exactly `test/expected.txt`:

```
parent 15: 2 products, 6 cells 10.00 GeV, 2 shared cells 3.00 GeV, shared energy fraction 0.30
  product -211: 4 cells 4.00 GeV, 2 shared 1.00 GeV, purity 0.67, nearest product 111 at dR 0.10, Delta R label lost, hit label kept, same label no
  product 111: 4 cells 6.00 GeV, 2 shared 2.00 GeV, purity 0.86, nearest product -211 at dR 0.10, Delta R label lost, hit label kept, same label no
  labels kept: Delta R 0 of 2, hits 2 of 2
parent 6: 2 products, 3 cells 7.75 GeV, 1 shared cells 3.50 GeV, shared energy fraction 0.45
  product 5: 2 cells 7.00 GeV, 1 shared 3.00 GeV, purity 0.93, nearest product 24 at dR 0.30, Delta R label lost, hit label kept, same label no
  product 24: 2 cells 0.75 GeV, 1 shared 0.50 GeV, purity 0.20, nearest product 5 at dR 0.30, Delta R label lost, hit label lost, same label yes
  labels kept: Delta R 0 of 2, hits 1 of 2
parent -6: 2 products, 2 cells 3.00 GeV, 0 shared cells 0.00 GeV, shared energy fraction 0.00
  product -5: 1 cells 2.00 GeV, 0 shared 0.00 GeV, purity 1.00, nearest product -24 at dR 1.00, Delta R label kept, hit label kept, same label yes
  product -24: 1 cells 1.00 GeV, 0 shared 0.00 GeV, purity 1.00, nearest product -5 at dR 1.00, Delta R label kept, hit label kept, same label yes
  labels kept: Delta R 2 of 2, hits 2 of 2
parent -15: 1 products, 0 cells 0.00 GeV, 0 shared cells 0.00 GeV, shared energy fraction 0.00
  product 11: 0 cells 0.00 GeV, 0 shared 0.00 GeV, purity 0.00, no other product, Delta R label kept, hit label lost, same label no
  labels kept: Delta R 1 of 1, hits 0 of 1
```

The first parent is the use case. The pi- and the pi0 of a tau are 0.10 apart, so a Delta R match loses both labels. They share 2 of 6 cells and 30% of the energy, and each one owns most of the energy in its cells, so the hits keep both labels. The tests state every number of the four parents, calculated by hand.

## What the test does not exercise

- The tests run the hit logic only. They do not run the selection of the parents and of the products on a graph, and they do not read a hit index. On five events of the ttbar tutorial sample the C++ plugin and `boosted.py` print the same lines for taus and for tops, but no test repeats this.
- The ttbar tutorial sample has no boosted top: the b and the W of each of the ten tops in the five events are more than 1.0 apart. A boosted ditau or ditop sample is necessary for the measurement.
