#!/bin/bash

cd standalone_aggregate_robinhood_mult
/bin/bash run-benchmark.sh $1
cd $1
for file in *
do 
	python ../../process.py $file
done
cd ../..


cd standalone_aggregate_unorderedmap_mult
/bin/bash run-benchmark.sh $1
cd $1
for file in *
do 
	python ../../process.py $file
done
cd ../..