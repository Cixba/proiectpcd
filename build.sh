#!/bin/bash

set -x

#creare folder de build

mkdir -p build

cd build

#generare cmake
cmake ..

#compilare
make
