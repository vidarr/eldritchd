#!/bin/bash

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
SRC_DIR=$SCRIPT_DIR/..

DEST_DIR=$1

[ "X$DEST_DIR" == "X" ] && DEST_DIR=$SRC_DIR

cd $DEST_DIR

[ -d release ] || mkdir release
cd release && cmake -DCMAKE_BUILD_TYPE=Release $SRC_DIR && cd ..
[ -d debug ]   || mkdir debug
cd debug && cmake -DCMAKE_BUILD_TYPE=Debug $SRC_DIR && cd ..

cd $ORIGINAL_DIR

