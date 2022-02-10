#!/bin/sh
set -x
set -e


# Dependencies
##python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m windows/qt-dependencies.tt.manifest
mkdir fetches && cd fetches
wget -q https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/EAtDCYYTTXaY4p3JDwovLQ/runs/0/artifacts/public/qt_src.zip || true
wget -q https://s3.nl-ams.scw.cloud/basti.data/vs_studio_sdk.zip || true
unzip -o -qq vs_studio_sdk.zip || true
unzip -o -qq qt_src.zip|| true
cd ../
ls -ial


powershell.exe -ExecutionPolicy Bypass taskcluster/scripts/toolkit/compile_qt_6.ps1   

python3 taskcluster/scripts/toolkit/test.py
