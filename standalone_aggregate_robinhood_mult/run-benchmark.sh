#!/bin/bash
###########################Change this snippet

#Run queries
logGroupSizes=(6) #(2 3 4 5 6 7 8 9 10) # 4 8 16 32 64 128 256 512 1024)
tuplesize=8
size=$1
thread=40


mkdir $1
for skew in 0 ; do
	datasize=$(($size*1000*1000*1000))
	numOfTuples=$(($datasize/$tuplesize))
	echo "" > $1/raw_results_${size}_${skew}.txt
	for logGroupSize in ${logGroupSizes[@]} ; do
		groupSize=$(echo "2^${logGroupSize}" | bc)
		make clean -C ../aggregate_robinhood_mult/
		#echo $logGroupSize
		#echo $groupSize
		make logGroups=${logGroupSize} -C ../aggregate_robinhood_mult/	

		../aggregate_robinhood_mult/aggregate /slowdisk/$size/$skew/data_${groupSize}.tbl $thread $groupSize $numOfTuples &>> $1/raw_results_${size}_${skew}.txt

		echo "" >> $1/raw_results_${size}_${skew}.txt
	done
	echo "${size}_${skew} finished"
done
