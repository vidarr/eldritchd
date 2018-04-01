#!/bin/bash
USER=httpd
GROUP=httpd
EXECUTABLE=eldritchd
VERSION=release
RC_SOURCE=../etc/eldritch.rc

function parse_arguments {
    if [ ! -z $1 ]; then
        VERSION=$1
    fi
}

function check_root {
    if [ $(id -u) != 0 ]; then
        echo "Must be run as root"
        exit 1
    fi
}

function setup_user {
    $(id $USER &> /dev/null)
    if [ $? == 0 ]; then
        echo "User $USER exists"
        # exit 1
    else
        groupadd $GROUP
        useradd $USER -m -g $GROUP
    fi
    HOME_DIR=/home/$USER
}

function setup_executable {
    SOURCE=../${VERSION}/src/$EXECUTABLE
    echo "Installing $SOURCE to $HOME_DIR/bin"
    chown root:root $HOME_DIR
    mkdir -p $HOME_DIR/bin
    chown root:root $HOME_DIR
    cp $SOURCE $HOME_DIR/bin
    chmod u+s $HOME_DIR/bin/$EXECUTABLE
    chmod g+s $HOME_DIR/bin/$EXECUTABLE
}

function setup_root_directory {
    mkdir -p $HOME_DIR/htdocs
    chown $USER:$GROUP $HOME_DIR/htdocs
}

function install_default_config {
  RC_TARGET="${HOME_DIR}/${EXECUTABLE}.rc"
  if [ -e "$RC_TARGET" ]; then
    echo "$RC_TARGET already in place - not overwriting it"
  else
    echo "Installing $RC_SOURCE to $RC_TARGET"
    cp $RC_SOURCE $RC_TARGET
    chown root:root $RC_TARGET
  fi
}


parse_arguments $*
check_root
setup_user
setup_executable
setup_root_directory
install_default_config
