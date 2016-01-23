#!/bin/bash
USER=httpd
GROUP=httpd

userdel $USER
groupdel $GROUP
rm /home/$USER -r
