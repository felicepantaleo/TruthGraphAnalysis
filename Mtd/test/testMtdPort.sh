#!/bin/bash
# Original author: Felice Pantaleo (CERN) <felice.pantaleo@cern.ch>

# Regression test of the MTD port against MtdTracksValidation. One job runs both modules
# on the first events of the ttbar tutorial sample, read over https. The harvested output
# goes to compareMtdPort.py, and the table must equal expected_compare.txt exactly.
# The test fails when the input cannot be read, when a job fails, or after the deadline.
# MTD_PORT_TEST_INPUT names another copy of the same file, for example a local one.
set -euo pipefail

TEST_DIR="${CMSSW_BASE}/src/TruthGraphAnalysis/Mtd/test"
INPUT="${MTD_PORT_TEST_INPUT:-https://felice.web.cern.ch/orbit/TruthGraphTutorial/ttbar/step3.root}"
EVENTS=20
DEADLINE=900 # seconds, for each cmsRun job

WORK="$(mktemp -d)"
trap 'rm -rf "${WORK}"' EXIT
cd "${WORK}"

timeout "${DEADLINE}" cmsRun "${TEST_DIR}/mtdGraphOnStep3_cfg.py" -i "${INPUT}" -n "${EVENTS}" --legacy -o mtdBoth_inDQM.root
timeout "${DEADLINE}" cmsRun "${TEST_DIR}/harvestMtd_cfg.py" -i mtdBoth_inDQM.root
python3 "${TEST_DIR}/compareMtdPort.py" DQM_V0001_R000000001__TruthGraphAnalysis__Mtd__Port.root >compare.txt
diff -u "${TEST_DIR}/expected_compare.txt" compare.txt
echo "The legacy and the graph columns match expected_compare.txt."
