#!/bin/bash
###########################Change this snippet

#Run queries
groupSizes=( 4 8 16 32 64 128) #256 512) # 1024)
tuplesize=8
size=$1

make clean -C ../aggregate_unorderedmap_murmur/
make -C ../aggregate_unorderedmap_murmur/

mkdir $1
for skew in 0; do
	datasize=$(($size*1024*1000*1000))
	numOfTuples=$(($datasize/$tuplesize))
	echo "" > $1/raw_results_${size}_${skew}.txt
	for groupSize in ${groupSizes[@]} ; do

		/fastdisk/aggregate_unorderedmap_murmur/aggregate /slowdisk/$size/$skew/data_${groupSize}.tbl 40 $groupSize $numOfTuples &>> $1/raw_results_${size}_${skew}.txt

		echo "" >> $1/raw_results_${size}_${skew}.txt
	done
	echo "${size}_${skew} finished"
done