# TruthGraphAnalysis

Worked analyses on the CMS MC-truth graph (`truth::Graph`, cms-sw/cmssw PR #51829): one
per selection preset of `PhysicsTools/TruthInfo/python/truthGraphSelections.py`, plus one
on the tau levels, which need no preset. Every example exists twice, in C++ and in python,
prints the same lines on the same event, and is tested against a committed graph of one
event.

| example | preset | question |
|---|---|---|
| [Gun](Gun/README.md) | `gun` | Per gun particle: how many reconstructable products does it have, and how many of its descendants reach the calorimeter? |
| [Resonance](Resonance/README.md) | `resonance` | Which two leptons does the Z decay to, and does their invariant mass reproduce the generator mass of the Z? |
| [Vbf](Vbf/README.md) | `vbf` | What are the dijet mass and the rapidity gap of the two quarks that recoil against the Higgs? |
| [Ggf](Ggf/README.md) | `ggf` | What does the detector see of the Higgs: how many reconstructable products, and which fraction of its energy is visible? |
| [Vh](Vh/README.md) | `vh` | Which vector boson was produced together with the Higgs, and how did it decay? |
| [Top](Top/README.md) | `top` | For each top: is there a b, how did the W decay, how large is its subgraph; and what class is the event, all hadronic, semileptonic or dilepton? A W to tau nu counts as leptonic. |
| [SingleTop](SingleTop/README.md) | `singletop` | What was produced together with the top: the recoil quark, the associated W, or the b? |
| [Diboson](Diboson/README.md) | `diboson` | What is the mass of the boson pair, and how did each boson decay? |
| [HeavyFlavor](HeavyFlavor/README.md) | `heavyflavor` | How far does each b hadron fly before it decays, and how many charm hadrons does it leave below it? |
| [Full](Full/README.md) | `full` | What does the whole event hold, interaction by interaction, and how large is the reconstructable final state of the signal and of the pile-up? |
| [Tau](Tau/README.md) | none, levels only | How did each tau decay: how many charged hadrons and neutral pions, which decay mode tau reconstruction would call it, and how much of the tau energy is visible? |

## Layout

Each example is a CMSSW package, so `scram` builds and tests it:

```
<Example>/
  README.md              the physics question, how to run, the expected output
  cpp/<Example>.h        run(graph, out): the analysis as a free function
  cpp/<Example>.cc
  cpp/<Example>Plugin.cc an EDAnalyzer that runs it in cmsRun
  py/<example>.py        the same analysis with PhysicsTools.TruthInfo.graphTools
  plugins/BuildFile.xml  builds cpp/ as the plugin library
  test/                  the C++ and the python test, and expected.txt they both match
```

`Common/` holds `GraphFromJson`, which builds a `truth::Graph` from the JSON that
`TruthLogicalGraphDumper` writes, and the fixtures: one dumped event per example under
`Common/fixtures/`. The tests run every example on its fixture and compare the output,
character by character, with `test/expected.txt`, in both languages.

## Where the two languages read the levels

The C++ recomputes a level from the graph (`truth::particlesAtLevel`, `truth::particlesAtLevels`,
`truth::branchesAtLevel`). Python has no level algorithm, so it reads the flags stamped on each
particle when the file was written. On a file written by the current release the two agree,
and the tests prove it on every fixture. On a file written before a level definition changed,
they differ, and the C++ is the current definition. For example, `bHadrons` kept the first B
hadron of a chain before 2026-08-20 and keeps the weakly decaying one since.

## Setup

The examples read the graph through the interface of branch `truth-adaptive-associator-v1`
(felicepantaleo/cmssw), which is PR #51829. In a CMSSW area:

```bash
cmsrel CMSSW_20_1_X_2026-09-13-2300 && cd CMSSW_20_1_X_2026-09-13-2300/src && cmsenv
git cms-init
git cms-merge-topic felicepantaleo:truth-adaptive-associator-v1
git clone git@github.com:felicepantaleo/TruthGraphAnalysis.git TruthGraphAnalysis
scram b -j 8
scram b runtests
```

## Running an example

```bash
# C++: a plugin, on a file that holds the logical graph. A production file holds the
# graph with no selection preset, so --rebuild builds it again with the preset from the
# GEN and SIM record in the file. A gun takes its species from the fragment name.
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Top --rebuild
cmsRun TruthGraphAnalysis/Common/test/runExample_cfg.py step3.root --example Gun --rebuild --fragment TenTau_E_15_500

# python: on a JSON dump (TruthLogicalGraphDumper with jsonFile set), or on an EDM file
python3 TruthGraphAnalysis/Top/py/top.py TruthGraphAnalysis/Common/fixtures/top.json
python3 TruthGraphAnalysis/Top/py/top.py step3.root
```

## Running on your own sample

A production file holds the graph with no selection preset, so the signal level is empty
in it. To build the graph again with a preset, over a GEN-SIM sample that already exists,
name the preset or name the generator fragment it came from:

```bash
cmsDriver.py step3 --filein file:step2.root ... \
  --customise_commands "from PhysicsTools.TruthInfo.customiseTruthPreset import applyTruthPreset; applyTruthPreset(process, preset='top')"

# the same thing with the fragment name, which the rules resolve to a preset
TRUTH_GRAPH_FRAGMENT=TTbar_14TeV_TuneCP5_cfi cmsDriver.py step3 ... \
  --customise PhysicsTools/TruthInfo/customiseTruthPreset.customiseTruthPreset
```

The job prints the preset it resolved and its seed species, so the log says which view was
built. One call is enough because the preset decides which particle is the signal, and the
graph producer and the truth-side targets producer both have to read the same answer.
`Common/test/runExample_cfg.py` uses the same call under `--rebuild`.

To see which preset a fragment maps to before running anything:

```bash
python3 $CMSSW_BASE/src/PhysicsTools/TruthInfo/python/truthGraphSelections.py \
    TTbar_14TeV_TuneCP5_cfi --name
```

## Code style

`.clang-format` and `.clang-tidy` are the ones of the CMSSW release, so
`scram build code-format` and `scram build code-checks` apply as in any CMSSW package.
The CI checks the format on every push and runs the python tests without CMSSW; the
CMSSW job builds the packages against the branch, runs clang-tidy and every test.
