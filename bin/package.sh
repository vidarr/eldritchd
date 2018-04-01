#!/usr/bin/bash
PACKAGE_DIR=eldritch
BIN_FILES_TO_PACKAGE="setup.sh uninstall.sh"

cd release && make buildnum && make
cd -

[ -d $PACKAGE_DIR ] || mkdir $PACKAGE_DIR
[ -d $PACKAGE_DIR/release ] || mkdir $PACKAGE_DIR/release
[ -d $PACKAGE_DIR/release/src ] || mkdir $PACKAGE_DIR/release/src
[ -d $PACKAGE_DIR/bin ] || mkdir $PACKAGE_DIR/bin

cp release/src/eldritchd ${PACKAGE_DIR}/release/src

cp etc ${PACKAGE_DIR} -r

for f in $BIN_FILES_TO_PACKAGE; do
    cp bin/$f ${PACKAGE_DIR}/bin
done

tar cf eldritch.tar.gz $PACKAGE_DIR
