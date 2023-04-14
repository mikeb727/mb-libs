#!/bin/sh
glad --api gl:core=3.3 --out-path glad_tmp c
mv -n glad_tmp/include/* include
mv -n glad_tmp/src/* src
rm -rf ./glad_tmp
