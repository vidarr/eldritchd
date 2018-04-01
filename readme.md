# Overview

eldritchd is a simple, minimalistic HTTP daemon.

It has been written in plain C and facilitates only Posix and the
BSD socket API.

If you want to experience Eldritch in action, visit

http://ubeer.org/eldritch.html

Not accepting connections? - Well, Eldritch is still under development ;)

# Requirements

Eldritch requires a posix compatible operating system, along with
BSD sockets available, as well as  [CMake](https://cmake.org).
A Bash compatible shell is required for executing the scripts.

# Howto Build

Eldritch uses [CMake](https://cmake.org).

First of all, create the necessary build directories by calling

```Bash
bin/prepare-builds.sh
```

then change into the directory `release` or `debug` and call

```Bash
cmake ..
```

This will create Makefiles to compile and link the executable.
Now you can enter

```Bash
make
```

and the executable will be built underneath `src` .

Finally, the executable needs to be installed since some precaution has to be
taken in order to execute properly (like setuid root etc).

Get back into the main directory and issue

```Bash
bin/setup.sh
```

or

```Bash
bin/setup.sh debug
```

if you compiled the `debug` version.

Eldritch should now be installed underneath `/home/httpd/` as user `httpd`
(this user will be created by the setup script if not present yet).

# Howto Start

Eldritch is fairly simple designed.

It requires a config file to start up, like this one:

```
listen * 80
logfile /tmp/eldritch.log
documentroot /home/httpd/htdocs
contenttype zip application/zip
```

Thus create a file with this content and save it.

Become user `httpd` by

```Bash
su httpd
```

And change directories into the home directory of this user

```Bash
cd $HOME
```

Eldritch can now be started via

```Bash
bin/eldritchd [PATH_TO_CONFIG_FILE]
```

If `PATH_TO_CONFIG_FILE` is omitted, eldritch will try to load a default config
file.

Now eldritch will start up, load the config, try to bind to the interfaces
specified in the config, chroot to its document root, drop root priviledges and
deamonize.

# Using Eldritch

the configuration of eldritch is straight forward.

You provide all configuration in a single file, and hand over  the path to this 
file as command line argument to eldritch.

The file itself should contain an entry

* `listen INTERFACE PORT` : Eldritch will bind to the socket INTERFACE:PORT
* `logfile PATH_TO_LOGFILE`: Eldritch will log into this file.
* `documentroot PATH_TO_DIRECTORY`: Eldritch will chroot to this directory and\
 use it as its 'database' - i.e. provide the files therein.

By default, eldritch only delivers `text/html` for any file.
You might define additional mime types for certain file name extensions by

* `contenttype FILE_EXTENSION MIME_TYPE`: Eldritch will deliver any file \
 `*.EXTENSION_TYPE` with `MIME_TYPE` as mime.


Eldritch will use `/home/httpd/htdocs` as document root.
If you request `/index.html` from eldritch, it will look for a file
`/home/httpd/htdocs/index.html` etc.

# Demo

If you want to experience Eldritch in action, visit

[http://ubeer.org/eldritch.html](http://ubeer.org/eldritch.html)

Not accepting connections? - Well, Eldritch is still under development ;)

# Licensing

With the exception of CuTest ( http://cutest.sourceforge.net ), all code

 (C) 2016 Michael J. Beer
 All rights reserved.

 Redistribution  and use in source and binary forms, with or with‐
 out modification, are permitted provided that the following  con‐
 ditions are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above  copy‐
 right  notice,  this  list  of  conditions and the following dis‐
 claimer in the documentation and/or other materials provided with
 the distribution.

 3.  Neither the name of the copyright holder nor the names of its
 contributors may be used to endorse or promote  products  derived
 from this software without specific prior written permission.

 THIS  SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBU‐
 TORS "AS IS" AND ANY EXPRESS OR  IMPLIED  WARRANTIES,  INCLUDING,
 BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 FITNESS FOR A PARTICULAR PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT
 SHALL  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DI‐
 RECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS IN‐
 TERRUPTION)  HOWEVER  CAUSED  AND  ON  ANY  THEORY  OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  NEGLI‐
 GENCE  OR  OTHERWISE)  ARISING  IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

CuTest (C) Asim Jalis, is distributed under the zlib/libpng license
 ( http://opensource.org/licenses/zlib-license.html ).
