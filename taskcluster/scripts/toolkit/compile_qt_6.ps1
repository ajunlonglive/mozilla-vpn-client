
echo("Available SDKS:")
 Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\Include"
# Enter the DEV Shell
. "$PSScriptRoot/enter_dev_shell.ps1"


if(!(Test-Path QTBIN)){
  New-Item -Path QTBIN -ItemType "directory"
}

exit 0;
# Enter QT source directory
Set-Location fetches/qt-everywhere-src-6.2.3/

./configure `
  -static  `
  -opensource  `
  -release  `
  -no-dbus   `
  -no-feature-qdbus  `
  -confirm-license  `
  -strip  `
  -silent  `
  -nomake tests  `
  -nomake examples  `
  -make libs  `
  -no-sql-psql  `
  -qt-sqlite  `
  -skip qt3d  `
  -skip webengine  `
  -skip qtmultimedia  `
  -skip qtserialport  `
  -skip qtsensors  `
  -skip qtgamepad  `
  -skip qtwebchannel  `
  -skip qtandroidextras  `
  -feature-imageformat_png  `
  -qt-libpng  `
  -qt-zlib  `
  -prefix ../../QT_OUT
# TODO: openssl dep so we can link it. 
#  -openssl-linked `
#  -I $BUILD_TOOLS_DIR\include `
#  -L $BUILD_TOOLS_DIR\lib `
 # -prefix QTBIN `

 cmake --build . --parallel

 cmake --install .