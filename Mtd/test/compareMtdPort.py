#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""Compare the graph port of the MTD track validation with MtdTracksValidation.

Reads one harvested DQM file that holds both folders, MTD/Tracks and MTD/TracksGraph, and
prints the track count of every category side by side, under the monitor element names
the two modules share.

    python3 compareMtdPort.py DQM_V0001_R000000001__Global__CMSSW_X_Y_Z__RECO.root
"""

import sys

import ROOT

LEGACY = "MTD/Tracks"
GRAPH = "MTD/TracksGraph"

# (label, monitor element name in both folders)
ROWS = {
    "BTL": [
        ("tracks matched to truth", "BTLTrackMatchedTPEtaTot"),
        ("  particle with direct hits", "BTLTrackMatchedTPmtdDirectEta"),
        ("    first direct cluster on the track", "BTLTrackMatchedTPmtdDirectCorrectAssocEta"),
        ("    another cluster on the track", "BTLTrackMatchedTPmtdDirectWrongAssocEta"),
        ("      a later direct cluster", "BTLTrackMatchedTPmtdDirectWrongAssocEta1"),
        ("      an other cluster of it", "BTLTrackMatchedTPmtdDirectWrongAssocEta2"),
        ("      not its cluster", "BTLTrackMatchedTPmtdDirectWrongAssocEta3"),
        ("    no cluster on the track", "BTLTrackMatchedTPmtdDirectNoAssocEta"),
        ("  particle with other hits only", "BTLTrackMatchedTPmtdOtherEta"),
        ("    its cluster on the track", "BTLTrackMatchedTPmtdOtherCorrectAssocEta"),
        ("    another cluster on the track", "BTLTrackMatchedTPmtdOtherWrongAssocEta"),
        ("    no cluster on the track", "BTLTrackMatchedTPmtdOtherNoAssocEta"),
        ("  particle without MTD hits", "BTLTrackMatchedTPnomtdEta"),
    ],
    "ETL": [
        ("tracks matched to truth", "ETLTrackMatchedTPEtaTot"),
        ("  particle with MTD hits", "ETLTrackMatchedTPmtd1Eta"),
        ("    correct cluster", "ETLTrackMatchedTPmtd1CorrectAssocEta"),
        ("    wrong cluster", "ETLTrackMatchedTPmtd1WrongAssocEta"),
        ("    no cluster", "ETLTrackMatchedTPmtd1NoAssocEta"),
        ("  particle without MTD hits", "ETLTrackMatchedTPnomtdEta"),
    ],
}

TIME_RES = [
    ("BTL", "BTLTrackMatchedTPmtdDirectCorrectAssocTimeRes"),
    ("ETL", "ETLTrackMatchedTPmtd1CorrectAssocTimeRes"),
]


def histogram(tfile, folder, name):
    h = tfile.Get(f"DQMData/Run 1/{folder.split('/')[0]}/Run summary/{'/'.join(folder.split('/')[1:])}/{name}")
    if not h:
        raise KeyError(f"{folder}/{name} is not in the file")
    return h


def entries(tfile, folder, names):
    return int(sum(histogram(tfile, folder, n).GetEntries() for n in names))


def main(path):
    tfile = ROOT.TFile.Open(path)
    for region, rows in ROWS.items():
        print(f"{region:<40} {'legacy':>8} {'graph':>8} {'graph/legacy':>13}")
        for label, name in rows:
            # A release can lack a monitor element in one of the two modules.
            try:
                a = entries(tfile, LEGACY, [name])
                b = entries(tfile, GRAPH, [name])
            except KeyError:
                print(f"{label:<40} {'n/a':>8} {'n/a':>8}")
                continue
            ratio = f"{b / a:13.3f}" if a else f"{'-':>13}"
            print(f"{label:<40} {a:8d} {b:8d} {ratio}")
        print()
    print(f"{'time residual, correct cluster':<34} {'legacy mean':>12} {'rms':>7} {'graph mean':>12} {'rms':>7}  [ps]")
    for region, name in TIME_RES:
        a = histogram(tfile, LEGACY, name)
        b = histogram(tfile, GRAPH, name)
        print(f"  {region:<32} {a.GetMean() * 1e3:12.1f} {a.GetRMS() * 1e3:7.1f} "
              f"{b.GetMean() * 1e3:12.1f} {b.GetRMS() * 1e3:7.1f}")


if __name__ == "__main__":
    main(sys.argv[1])
