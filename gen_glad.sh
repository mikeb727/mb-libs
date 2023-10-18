#!/bin/sh
glad --api gl:core=3.3 --out-path glad_tmp c
glad --api glx:core=1.4 --out-path glad_tmp c
mv -i glad_tmp/include/* include
mv -i glad_tmp/src/* src
rm -rf ./glad_tmp
