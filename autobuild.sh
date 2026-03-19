#!/bin/bash 
set -e
ROOT_DIR=$(pwd)
BUILD_DIR=$ROOT_DIR/build
LIB_DIR=$ROOT_DIR/knight_lib

mkdir -p $BUILD_DIR
mkdir -p $LIB_DIR

cd $BUILD_DIR
cmake ..
make -j$(nproc)

cd $ROOT_DIR
find $BUILD_DIR -name "*.a" -exec cp {} $LIB_DIR \;
