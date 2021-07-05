# Mozilla VPN

>_One tap to privacy
Surf, stream, game, and get work done while maintaining your privacy online. Whether you’re traveling, using public WiFi, or simply looking for more online security, we will always put your privacy first._ 

See: https://vpn.mozilla.org

## Getting Involved

We encourage you to participate in this open source project. We love Pull Requests, Bug Reports, ideas, (security) code reviews or any other kind of positive contribution. 

Before you attempt to make a contribution please read the [Community Participation Guidelines](https://www.mozilla.org/en-US/about/governance/policies/participation/).

* [View current Issues](https://github.com/mozilla-mobile/mozilla-vpn-client/issues), [view current Pull Requests](https://github.com/mozilla-mobile/mozilla-vpn-client/pulls), or [file a security issue](https://bugzilla.mozilla.org/enter_bug.cgi?product=Mozilla%20VPN).
* Localization happens on [Pontoon](https://pontoon.mozilla.org/projects/mozilla-vpn-client/).
* Matrix [#mozilla-vpn:mozilla.org channel](https://chat.mozilla.org/#/room/#mozilla-vpn:mozilla.org)
* [View the Wiki](https://github.com/mozilla-mobile/mozilla-vpn-client/wiki).
* [View the Support Docs](https://support.mozilla.org/en-US/products/firefox-private-network-vpn)


## Dev instructions

After checking out the code:

* Install the git pre-commit hook (`./scripts/git-pre-commit-format install`)
* Build the source (See below)
* Run the tests `./scripts/test_coverage.sh` or `./scripts/test_function.sh`

## How to build from the source code

### Linux

On linux, the compilation of MozillaVPN is relative easy. You need the
following dependencies:

- Qt5 >= 5.11
- libpolkit-gobject-1-dev >=0.105
- wireguard >=1.0.20200513
- wireguard-tools >=1.0.20200513
- resolvconf >= 1.82
- golang >= 1.13

Python3 (pip) depedencies:

- glean_parser==3.5
- pyhumps
- pyyaml

#### QT5

Qt5 can be installed in a number of ways:
- download a binary package or the installer from the official QT website: https://www.qt.io/download
- use a linux package manager
- compile Qt5 (dinamically or statically).

On Ubuntu 20.04, Debian 10/buster, or later, the Qt development libraries can be installed from the
package repositories with the following steps:
```
sudo apt install qtbase5-dev qtbase5-dev-tools qttools5-dev-tools qt5-qmake-bin \
                 qtdeclarative5-dev qtdeclarative5-dev-tools \
                 libqt5charts5-dev libqt5networkauth5-dev libqt5svg5-dev libqt5websockets5-dev \
                 qml-module-qtcharts qml-module-qtgraphicaleffects \
                 qml-module-qtquick-controls qml-module-qtquick-controls2 \
                 qml-module-qtquick-extras qml-module-qtquick-layouts qml-module-qtquick-window2 \
                 qml-module-qtquick2 qml-module-qtqml-models2
```

#### Initialization

```
# submodules
git submodule init
git submodule update
# glean
./scripts/generate_glean.py
# translations
./scripts/importLanguages.py
```

#### Build

To build next to source:

```
qmake CONFIG+=production
make -j8 # replace 8 with the number of cores. Or use: make -j$(nproc)
sudo make install
```

For local dev builds, the following qmake command may be more helpful `qmake CONFIG+=debug CONFIG+=inspector`.

If you prefer to not install at /usr or /etc, you can specify alternate prefixes. Using no prefixes is equivalent to:

```
qmake USRPATH=/usr ETCPATH=/etc CONFIG+=production
```

#### Run

If you have built into /usr, simply run

```
mozillavpn
```

Alternatively, you can use two terminals to run the daemon manually and seperately e.g.

```
sudo mozillavpn linuxdaemon
mozillavpn
```

mozillavpn linuxdaemon needs privileged access and so if you do not run as root, you will get an authentication prompt every time you try to reconnect the vpn.


### MacOS

On macOS, we strongly suggest to compile Qt5 statically. To do that, follow these steps:

```
curl -L https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz --output qt-everywhere-src-5.15.1.tar.xz
tar vxf qt-everywhere-src-5.15.1.tar.xz
mv qt-everywhere-src-5.15.1 qt
bash scripts/qt5_compile.sh `pwd`/qt qt
export QT_MACOS_BIN=`pwd`/qt/qt/bin
```

The procedure to compile MozillaVPN for macOS is the following:

1. Install XCodeProj:
```
  $ [sudo] gem install xcodeproj
```
2. Install go if you haven't done it before: https://golang.org/dl/
3. Update the submodules:
```
  $ git submodule init
  $ git submodule update --remote
```
4. Install python3 dependencies:
```
  $ pip3 install 'glean_parser==3.5'
  $ pip3 install pyhumps
  $ pip3 install pyyaml
```
5. Run the script (use QT\_MACOS\_BIN env to set the path for the Qt5 macos build bin folder):
```
  $ ./scripts/apple_compile.sh macos
```
6. Copy `xcode.xconfig.template` to `xcode.xconfig`
```
  $ cp xcode.xconfig.template xcode.xconfig
```
7. Modify xcode.xconfig to something like:
```
DEVELOPMENT_TEAM = 43AQ936H96

# MacOS configuration
GROUP_ID_MACOS = <>
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login
NATIVEMESSAGING_ID_MACOS = org.mozilla.macos.FirefoxVPN.native-messaging

# IOS configuration
GROUP_ID_IOS = <>
APP_ID_IOS = <>
NETEXT_ID_IOS = <>
```
8. Open Xcode and run/test/archive/ship the app

To build a Release style build (ready for signing), use:
```
cd MozillaVPN.xcodeproj
xcodebuild -scheme MozillaVPN -workspace project.xcworkspace -configuration Release clean build CODE_SIGNING_ALLOWED=NO
```

The built up will show up in `Release/Mozilla VPN.app` (relative to the root of the repo).

### IOS

The IOS procedure is similar to the macOS one:
1. Install XCodeProj:
```
  $ [sudo] gem install xcodeproj
```
2. Update the submodules:
```
  $ git submodule init
  $ git submodule update --remote
```
3. Install python3 dependencies:
```
  $ pip3 install 'glean_parser==3.5'
  $ pip3 install pyhumps
  $ pip3 install pyyaml
```
4. Copy `xcode.xconfig.template` to `xcode.xconfig`
```
  $ cp xcode.xconfig.template xcode.xconfig
```
5. Modify xcode.xconfig to something like:
```
DEVELOPMENT_TEAM = 43AQ936H96

# MacOS configuration
GROUP_ID_MACOS = <>
APP_ID_IOS = <>
NETEXT_ID_IOS = <>
LOGIN_ID_IOS = <>
NATIVEMESSAGING_ID_MACOS = <>

# IOS configuration
GROUP_ID_IOS = <>
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
```
6. Run the script (use QT\_IOS\_BIN env to set the path for the Qt5 ios build bin folder):
```
  $ ./scripts/apple_compile.sh ios
```
7. Open Xcode and run/test/archive/ship the app

### Android 

1. Install go if you haven't done it before: https://golang.org/dl/
2. Install Android SDK/NDK + JDK - https://doc.qt.io/qt-5/android-getting-started.html
3. We currently require NDK r20b and SDK >=21
4. Update the submodules:
```bash 
  $ git submodule init
  $ git submodule update --remote
```

5. Install python3 dependencies:
```
  $ pip3 install 'glean_parser==3.5'
  $ pip3 install pyhumps
  $ pip3 install pyyaml
```

6. Build the apk
```bash 
  $  ./scripts/android_package.sh /path/to/Qt/5.15.x/ (debug|release)
```
7. The apk will be located in ```.tmp/src/android-build//build/outputs/apk/debug/android-build-debug.apk```
8. Install with adb on device/emulator
```bash
  $ adb install .tmp/src/android-build//build/outputs/apk/debug/android-build-debug.apk
```

### Windows

We use a statically-compiled QT5.15 version to deploy the app. There are many
tutorials about to how to compile QT5 on windows, but to make this task
easier for everyone, there is a batch script to execute into a visual-studio
x86 context: `$ scripts\qt5_compile.bat`

The dependencies are:
1. perl: http://strawberryperl.com/
2. nasm: https://www.nasm.us/
3. python3: https://www.python.org/downloads/windows/
4. visual studio 2019: https://visualstudio.microsoft.com/vs/
5. Install python3 dependencies (pip install "glean_parser==3.5" pyyaml pyhumps)

Openssl can be obtained from here: https://www.openssl.org/source/
Qt5.15 can be obtained from: https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz

There is also a script to compile the application: `scripts\windows_compile.bat`

## Inspector

To build the inspector `qmake CONFIG+=debug CONFIG+=inspector`. Or use appropriate script parameters.

When running MozillaVPN, go to http://localhost:8766 to view the inspector.

From the inspector, type `help` to see the list of available commands.

## Bug report

Please file bugs here: https://github.com/mozilla-mobile/mozilla-vpn-client/issues

## Status

[![Unit Test Coverage](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_coverage.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_coverage.yaml)
[![Functional tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/functional_tests.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/functional_tests.yaml)
[![Linters (clang, l10n)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml)
[![Android](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/android.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/android.yaml)
[![MacOS](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/macos-build.yaml)
[![Windows](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/windows-build.yaml)
