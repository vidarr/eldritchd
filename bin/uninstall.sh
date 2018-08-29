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
cd $SCRIPT_DIR

USER=httpd
GROUP=httpd

userdel $USER
groupdel $GROUP
rm /home/$USER -r

cd $ORIGINAL_DIR
