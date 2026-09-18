# Fixtures

One event per example, as `TruthLogicalGraphDumper` writes it with `jsonFile` set, with the
selection preset of the example applied. The first five come from the local Run4 D122
samples of PR #51829 (2026-09-17). The last three come from Run4 D127 samples made with the
custom generator fragments in `PhysicsTools/TruthInfo/python`, because no relval workflow
covers these processes (2026-09-18):

| file | sample | event | selection |
|---|---|---|---|
| tentau.json | TenTau_E_15_500 | 2 | gun (seeds 15, -15, no spectators) |
| zee.json | ZEE | 56 | resonance (seeds 23, 32, decay group 11 -11) |
| vbf.json | VBFHZZ4Nu | 29 | vbf (seed 25, production siblings) |
| ggf.json | H125GGgluonfusion | 26 | ggf (seed 25) |
| top.json | TTbar | 9 | top (seeds 6, -6, production siblings) |
| vh.json | ZHToBB_ZToLL_14TeV_TuneCP5 | 4 | vh (seed 25, production siblings) |
| singletop.json | ST_tch_top_14TeV_TuneCP5 | 9 | singletop (seeds 6, -6, production siblings) |
| diboson.json | WWTo2L2Nu_14TeV_TuneCP5 | 5 | diboson (seeds 23, 24, -24, production siblings) |

`tentau.json` also serves the Tau example: its ten taus carry the tau levels, which need no
preset.
