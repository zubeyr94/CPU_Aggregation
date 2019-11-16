# CPU_Aggregation


#Data Generation
- With root privileges you should run datasetgen.sh, it takes two arguments $datasize in GB(1, 10 or 100) and $skew of data (0 for uniform 1.05 for low skewed and 1.25 for high skewed data). This script will create data files under /slowdisk folder.
    `./datasetgen.sh 10 0`

#Running the aggregation
- You should run `sudo apt-get install libbz2-dev` first.
- Then run run.sh and give the data size in GB as parameter.
    `./run.sh 10`
