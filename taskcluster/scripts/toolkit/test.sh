#!/bin/sh
set -x
set -e

# Dependencies
##python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m windows/qt-dependencies.tt.manifest


python3 -m pip list
python3 -m pip show certifi
python3 -m certifi

echo "---"
ls -ial

echo "---"

python3 taskcluster/scripts/toolkit/test.py




python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m taskcluster/manifests/vs2017-15.9.manifest
cd fetch
unzip vs2017_15.9.6.zip
cd ..
./taskcluster/scripts/misc/vs-setup.sh

where msbuild