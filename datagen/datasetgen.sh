#!/bin/bash

datasize=$(($1*1000*1000*1000))
tuplesize=8
numOfTuples=$(($datasize/$tuplesize))
skew=$2

sudo mkdir /slowdisk/$1
sudo mkdir /slowdisk/$1/$2

for numOfKeys in 128 #4 8 16 32 64 128 256 512 1024
do
	python3 datagen.py $numOfTuples $numOfKeys $skew > /slowdisk/$1/$2/data_${numOfKeys}.tbl
done
