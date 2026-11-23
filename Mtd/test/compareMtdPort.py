#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""Compare the graph port of the MTD track validation with MtdTracksValidation.

Reads one harvested DQM file that holds both folders, MTD/Tracks and MTD/TracksGraph, and
prints the track count of every category side by side, under the monitor element names
the two modules share, and the mean and the RMS of the time residual. A monitor element
that one folder does not hold prints as n/a. Mtd/test/testMtdPort.sh requires that the
output equals Mtd/test/expected_compare.txt.

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
    """The monitor element, or None when the folder does not hold it."""
    top, sub = folder.split("/", 1)
    h = tfile.Get(f"DQMData/Run 1/{top}/Run summary/{sub}/{name}")
    return h if h else None


def cell(value, width, fmt="d"):
    return f"{'n/a':>{width}}" if value is None else f"{value:{width}{fmt}}"


def main(path):
    tfile = ROOT.TFile.Open(path)
    for region, rows in ROWS.items():
        print(f"{region:<40} {'legacy':>8} {'graph':>8} {'graph/legacy':>13}")
        for label, name in rows:
            # A release can lack a monitor element in one of the two modules.
            a, b = (histogram(tfile, folder, name) for folder in (LEGACY, GRAPH))
            a = None if a is None else int(a.GetEntries())
            b = None if b is None else int(b.GetEntries())
            ratio = f"{b / a:13.3f}" if a and b is not None else f"{'-':>13}"
            print(f"{label:<40} {cell(a, 8)} {cell(b, 8)} {ratio}")
        print()
    print(f"{'time residual, correct cluster':<34} {'legacy mean':>12} {'rms':>7} {'graph mean':>12} {'rms':>7}  [ps]")
    for region, name in TIME_RES:
        columns = []
        for folder in (LEGACY, GRAPH):
            h = histogram(tfile, folder, name)
            columns.append(cell(None if h is None else h.GetMean() * 1e3, 12, ".1f"))
            columns.append(cell(None if h is None else h.GetRMS() * 1e3, 7, ".1f"))
        print(f"  {region:<32} {' '.join(columns[:2])} {' '.join(columns[2:])}")


if __name__ == "__main__":
    main(sys.argv[1])
