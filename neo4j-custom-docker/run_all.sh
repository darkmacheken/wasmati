#!/bin/bash

if [[ -z $1 ]]; then
    echo "$0: Required path for directory containing graph files"
    exit 4
fi

GRAPH_DIR_PATH=$(realpath $1)

for zip in $(ls $GRAPH_DIR_PATH)
do
    echo "###########################################"
    echo "[INFO] - Running for '$zip'"
    ./run_neo4j.sh $GRAPH_DIR_PATH/$zip
    echo "###########################################"
done