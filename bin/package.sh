#!/usr/bin/bash
PACKAGE_DIR=eldritch
BIN_FILES_TO_PACKAGE="setup.sh uninstall.sh"

function get_script_dir {

    # see https://stackoverflow.com/questions/11489428/how-to-make-vim-paste-from-and-copy-to-systems-clipboard
    SOURCE="${BASH_SOURCE[0]}"
    while [ -h "$SOURCE" ]; do
        DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
        SOURCE="$(readlink "$SOURCE")"
        [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
    done
    DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"

    echo $DIR

}


SCRIPT_DIR=$(get_script_dir)
ORIGINAL_DIR=$(pwd)
cd $SCRIPT_DIR/..


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

cd $ORIGINAL_DIR
