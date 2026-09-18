#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""On the fixture the example prints exactly test/expected.txt, which the C++ twin
matches too, so the two languages cannot drift apart."""

import io
import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "py"))

from PhysicsTools.TruthInfo.graphTools import TruthGraphView  # noqa: E402

import diboson  # noqa: E402


class TestDiboson(unittest.TestCase):
    def test_matches_the_expected_output(self):
        graph = TruthGraphView.fromJson(os.path.join(HERE, "..", "..", "Common", "fixtures", "diboson.json"))
        out = io.StringIO()
        diboson.run(graph, out)
        with open(os.path.join(HERE, "expected.txt"), encoding="utf-8") as handle:
            self.assertEqual(handle.read(), out.getvalue())


if __name__ == "__main__":
    unittest.main()
