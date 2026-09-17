# Fixtures

One event per example, as `TruthLogicalGraphDumper` writes it with `jsonFile` set, with the
selection preset of the example applied. Produced on 2026-09-17 from the local Run4 D122
samples of PR #51829:

| file | sample | event | selection |
|---|---|---|---|
| tentau.json | TenTau_E_15_500 | 2 | gun (seeds 15, -15, no spectators) |
| zee.json | ZEE | 56 | resonance (seeds 23, 32, decay group 11 -11) |
| vbf.json | VBFHZZ4Nu | 29 | vbf (seed 25, production siblings) |
| ggf.json | H125GGgluonfusion | 26 | ggf (seed 25) |
| top.json | TTbar | 9 | top (seeds 6, -6, production siblings) |
| top_diboson_seeds.json | TTbar | 9 | diboson (seeds 23, 24, -24, production siblings) |
