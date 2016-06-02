#!/bin/bash
[ -d release ] || mkdir release
cd release && cmake -DCMAKE_BUILD_TYPE=Release .. && cd ..
[ -d debug ]   || mkdir debug
cd debug && cmake -DCMAKE_BUILD_TYPE=Debug .. && cd ..

