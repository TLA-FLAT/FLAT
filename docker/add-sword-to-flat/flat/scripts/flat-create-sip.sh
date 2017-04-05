#!/bin/sh

BASE="`basename $1`"
BAG="bag-$BASE"

bag create "${BAG}" $1

zip -qr $BASE.zip $BAG

rm -rf $BAG

echo $BASE.zip