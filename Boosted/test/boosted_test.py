#!/usr/bin/env python3
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

"""On the hit lists below the hit logic prints exactly test/expected.txt, which the C++
twin matches too, so the two languages cannot drift apart. The numbers, by hand, with the
cone of 0.4:

tau 15. The pi- has 2.0 (two hits), 1.0, 0.5, 0.5 GeV in the cells 101 to 104. The pi0 has
1.5, 0.5, 3.0, 1.0 GeV in the cells 103 to 106. The cells 103 and 104 are shared: 2 of 6
cells, 2.0 + 1.0 = 3.0 of 10.0 GeV, fraction 0.30. Purity of the pi-:
4.0 / (2.0 + 1.0 + 2.0 + 1.0) = 0.67. Purity of the pi0: 6.0 / (2.0 + 1.0 + 3.0 + 1.0)
= 0.86. The axes are 0.10 apart: the Delta R label is lost twice, the hit label never.

top 6, boosted. The b has 4.0 and 3.0 GeV in the cells 201 and 202, the W has 0.5 and
0.25 GeV in 202 and 203. The cell 202 is shared: 1 of 3 cells, 3.5 of 7.75 GeV, fraction
0.45. Purities: 7.0 / 7.5 = 0.93 and 0.75 / 3.75 = 0.20, so the W loses its hit label.
The axes are 0.30 apart: the Delta R label is lost twice.

top -6, resolved. The b has 2.0 GeV in the cell 301, the W has 1.0 GeV in 302, and the
axes are 1.00 apart. No cell is shared, and both labels are kept twice.

tau -15. One electron with no hit: no other product, so the Delta R label is kept, and no
hit, so the hit label is lost.
"""

import io
import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "py"))

import boosted  # noqa: E402

TAU = [
    dict(pdgId=-211, eta=0.10, phi=0.20, hits=[(101, 1.5), (101, 0.5), (102, 1.0), (103, 0.5), (104, 0.5)]),
    dict(pdgId=111, eta=0.10, phi=0.30, hits=[(103, 1.5), (104, 0.5), (105, 3.0), (106, 1.0)]),
]
BOOSTED_TOP = [
    dict(pdgId=5, eta=0.00, phi=0.00, hits=[(201, 4.0), (202, 3.0)]),
    dict(pdgId=24, eta=0.00, phi=0.30, hits=[(202, 0.5), (203, 0.25)]),
]
RESOLVED_TOP = [
    dict(pdgId=-5, eta=1.00, phi=0.00, hits=[(301, 2.0)]),
    dict(pdgId=-24, eta=0.00, phi=0.00, hits=[(302, 1.0)]),
]
LEPTONIC_TAU = [dict(pdgId=11, eta=2.00, phi=1.00, hits=[])]


class TestBoosted(unittest.TestCase):
    def test_matches_the_expected_output(self):
        out = io.StringIO()
        boosted.report(15, TAU, boosted.CONE, out)
        boosted.report(6, BOOSTED_TOP, boosted.CONE, out)
        boosted.report(-6, RESOLVED_TOP, boosted.CONE, out)
        boosted.report(-15, LEPTONIC_TAU, boosted.CONE, out)
        with open(os.path.join(HERE, "expected.txt"), encoding="utf-8") as handle:
            self.assertEqual(handle.read(), out.getvalue())


if __name__ == "__main__":
    unittest.main()
