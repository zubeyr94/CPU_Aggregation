#!/bin/bash
# Load user-defined environment variables

set -e

echo "Loading settings from $1"
if ! source $1 ; then echo "Failed to load config" ; exit 1 ; fi

# Print some debug information about this test.
grep -v ^# $1

QS_ARGS_STORAGE="-storage_path="$QS_STORAGE

function load_data {
  # Creates a fresh load of the tpc-h data.
  if [ -d $DATA_PATH ] ; then
    rm -rf $QS_STORAGE
    QSEXE="$QS $QS_ARGS_BASE $QS_ARGS_STORAGE $QS_ARGS_NUMA_LOAD"

    # Use quickstep to generate the catalog file in a new folder.
    $QSEXE -initialize_db=true < $CREATE_SQL

    COUNTER=0
    for tblfile in $DATA_PATH/*.tbl* ; do
      # Resolve which table the file should be loaded into.
      TBL=""
      if [[ $tblfile == *"data_4"* ]]
      then
        TBL="data_4"
      elif [[ $tblfile == *"data_8"* ]]
      then
        TBL="data_8"
      elif [[ $tblfile == *"data_16"* ]]
      then
        TBL="data_16"
      elif [[ $tblfile == *"data_32"* ]]
      then
        TBL="data_32"
      elif [[ $tblfile == *"data_64"* ]]
      then
        TBL="data_64"
      elif [[ $tblfile == *"data_128"* ]]
      then
        TBL="data_128"
 #     elif [[ $tblfile == *"data_256"* ]]
 #     then
 #       TBL="data_256"
 #     elif [[ $tblfile == *"data_512"* ]]
 #     then
 #       TBL="data_512"
 #     elif [[ $tblfile == *"data_1024"* ]]
 #     then
 #       TBL="data_1024"
      fi

      echo Loading $TBL from file: $tblfile;
      if ! echo "COPY $TBL FROM '$tblfile' WITH (DELIMITER '|');" | $QSEXE;
      then
        echo "Quickstep load failed.";
        exit 1;
      fi

      let COUNTER=COUNTER+1
    done
    echo Done loading. Loaded $COUNTER files.
    $QSEXE <<< "\analyze"
    # Print the disk footprint of the newly created database
    CUT=" | cut -f 1"
    DBSIZE="du -m $QS_STORAGE"$CUT
    echo -n "Datatbase footprint in MB is: "
    eval $DBSIZE

  else
    echo "Data folder $DATA_PATH not found, quitting"
    exit
  fi
}

function run_queries {
  # Runs each TPC-H query several times.
  QSEXE="$QS $QS_ARGS_BASE $QS_ARGS_NUMA_RUN $QS_ARGS_STORAGE"
  TOTALRUNS=5
  queries=( 01 02 03 04 05 06 ) # 07 08 09 )
#  if [ "$QUERIES" != "ALL" ]; then
#    unset queries
#    read -r -a queries <<< "$QUERIES"
#    echo "Running a subset of queries with length ${#queries[@]}"
#  fi
  echo $QSEXE
  for query in ${queries[@]} ; do
    echo "Query $query.sql"
    if [ -f tmp.sql ] ; then
      rm tmp.sql &>/dev/null
    fi
    touch tmp.sql
    # Run each query a variable number of times.
    for i in `seq 1 $TOTALRUNS`;
    do
      cat $query.sql >> tmp.sql
    done
    timeout 15m $QSEXE < tmp.sql
    rc=$?
    if [ $rc = 124 ] ;
    then
      echo "Quickstep timed out on query $query, continuing to next query."
    elif [ $rc != 0  ] ;
    then
      echo "Quickstep failed on query $query, continuing to next query."
    fi
  done
  rm tmp.sql &>/dev/null
}

function analyze_tables {
  # Runs the analyze command on quickstep.
  QSEXE="$QS $QS_ARGS_BASE $QS_ARGS_NUMA_RUN $QS_ARGS_STORAGE"
  rm tmp.sql &>/dev/null
  touch tmp.sql
  echo "\analyze" >> tmp.sql
  if ! $QSEXE < tmp.sql ;
  then
    echo "Quickstep failed on analyze, exiting."
    exit 1
  fi
  rm tmp.sql &> /dev/null
}


if [ ! -x $QS ] ; then
  echo "Given Quickstep executable not found: $QS"
  echo "Specify it in quickstep.cfg."
  exit
fi

# Load data.
if [ $LOAD_DATA = "true" ] ; then
  load_data
  analyze_tables
fi

run_queries
