#!/bin/bash 

CMAKE=$(which cmake)
# echo $CMAKE

mkdir -p cmake-build-release 
$CMAKE -DCMAKE_BUILD_TYPE=Release -S . -B cmake-build-release 

$CMAKE --build cmake-build-release --target clean -j 4 
$CMAKE --build cmake-build-release --target all -j 4