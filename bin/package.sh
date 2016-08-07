#!/usr/bin/bash
PACKAGE_DIR=../eldritch

cd ../release && make buildnum && make
cd -

[ -d $PACKAGE_DIR ] || mkdir $PACKAGE_DIR
[ -d $PACKAGE_DIR/release ] || mkdir $PACKAGE_DIR/release
[ -d $PACKAGE_DIR/release/src ] || mkdir $PACKAGE_DIR/release/src

cd $PACKAGE_DIR
cp ../release/src/eldritchd release/src

cp ../etc . -r
cp ../bin . -r

tar cf ../eldritch.tar.gz $PACKAGE_DIR

