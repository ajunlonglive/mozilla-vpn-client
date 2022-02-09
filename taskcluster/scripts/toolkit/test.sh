#!/bin/sh
set -x
set -e

# Dependencies
##python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m windows/qt-dependencies.tt.manifest
mkdir zip && cd zip
wget -q https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/EAtDCYYTTXaY4p3JDwovLQ/runs/0/artifacts/public/qt_src.zip || true
wget -q https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/EnZvH5r1RjKVcGtk6SUbjQ/artifacts/public/vsstudio.zip|| true
unzip -o -qq vsstudio.zip|| true
unzip -o -qq qt_src.zip|| true


ls -ial


echo "---"
cd ../
echo "---"

python3 taskcluster/scripts/toolkit/test.py
